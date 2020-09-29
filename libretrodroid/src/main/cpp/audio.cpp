/*
 *     Copyright (C) 2019  Filippo Scognamiglio
 *
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "log.h"

#include "audio.h"
#include <cmath>

LibretroDroid::Audio::Audio(int32_t sampleRate) {
    LOGI("Audio initialization has been called with sample rate %d", sampleRate);

    // We are buffering a max of 125ms of audio.
    fifo = std::unique_ptr<oboe::FifoBuffer>(new oboe::FifoBuffer(2, sampleRate / 4));
    audioBuffer = std::unique_ptr<int16_t[]>(new int16_t[sampleRate / 4]);

    oboe::AudioStreamBuilder builder;
    builder.setChannelCount(2);
    builder.setDirection(oboe::Direction::Output);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setCallback(this);

    oboe::Result result = builder.openManagedStream(stream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
    }

    stream->open();

    defaultSampleRate = (double) sampleRate / stream->getSampleRate();
}

void LibretroDroid::Audio::start() {
    stream->requestStart();
}

void LibretroDroid::Audio::stop() {
    stream->requestStop();
}

void LibretroDroid::Audio::write(const int16_t *data, size_t frames) {
    size_t size = frames * 2;
    auto written = fifo->write(data, size);
    if (written != size) {
        LOGE("FILIPPO Buffer overrun detected");
    }
}

void LibretroDroid::Audio::resample_linear(const int16_t* source, int32_t inputFrames, int16_t* sink, int32_t sinkFrames) {
    double outputTime = 0;
    double outputTimeStep = 1.0 / sinkFrames;

    double floatingPart, integerPart;

    while (sinkFrames > 0) {
        floatingPart = std::modf(outputTime * inputFrames, &integerPart);

        int32_t floorFrame = integerPart;
        int32_t ceilFrame = std::min(floorFrame + 1, inputFrames - 1);

        *sink++ = source[ceilFrame * 2] * (floatingPart) + source[floorFrame * 2] * (1.0 - floatingPart);
        *sink++ = source[ceilFrame * 2 + 1] * (floatingPart) + source[floorFrame * 2 + 1] * (1.0 - floatingPart);
        outputTime += outputTimeStep;
        sinkFrames--;
    }
}

void LibretroDroid::Audio::resample_sinc(const int16_t* source, int32_t inputFrames, int16_t* sink, int32_t sinkFrames) {
    float outputTime = 0;
    float outputTimeStep = 1.0f / sinkFrames;

    while (sinkFrames > 0) {
        int32_t prevInputIndex = std::floor(outputTime * inputFrames);

        int32_t leftResult = 0;
        int32_t rightResult = 0;
        float gain = 0.05;

        auto startFrame = std::max(prevInputIndex - SINC_RESAMPLING_TAPS + 1, 0);
        auto endFrame = std::min(prevInputIndex + SINC_RESAMPLING_TAPS, inputFrames - 1);

        for (int32_t currentInputIndex = startFrame; currentInputIndex <= endFrame; currentInputIndex++) {
            float sincCoefficient = sinc((outputTime * (inputFrames)) - (double) currentInputIndex);
            gain += sincCoefficient;
            leftResult += source[currentInputIndex * 2] * sincCoefficient;
            rightResult += source[currentInputIndex * 2 + 1] * sincCoefficient;
        }

        outputTime += outputTimeStep;
        *sink++ = leftResult / gain;
        *sink++ = rightResult / gain;
        sinkFrames--;
    }
}

oboe::DataCallbackResult LibretroDroid::Audio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    double framesCapacityInBuffer = fifo->getBufferCapacityInFrames();
    double framesAvailableInBuffer = fifo->getFullFramesAvailable();
    double framesToMid = framesCapacityInBuffer - 2.0f * framesAvailableInBuffer;

    double sampleRateAdjustment = 1 - MAX_AUDIO_ACCELERATION * framesToMid / framesCapacityInBuffer;
    double finalSampleRate = defaultSampleRate * sampleRateAdjustment;

    int32_t adjustedTotalFrames = std::floor(numFrames * finalSampleRate);

    auto readFrames = fifo->readNow(audioBuffer.get(), adjustedTotalFrames * 2);
    if (readFrames != adjustedTotalFrames * 2) {
        LOGE("FILIPPO Buffer underrun detected");
    }

    auto outputArray = reinterpret_cast<int16_t *>(audioData);

    auto start = std::chrono::high_resolution_clock::now();
  //  resample_sinc(audioBuffer.get(), adjustedTotalFrames, outputArray, numFrames);
    resample_linear(audioBuffer.get(), adjustedTotalFrames, outputArray, numFrames);
    auto end = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    LOGI("FILIPPO Resampling time: %d", duration.count());

    return oboe::DataCallbackResult::Continue;
}

float LibretroDroid::Audio::sinc(float x) {
    if (abs(x) < 1.0e-9) return 1.0;
    return sinf(x * PI_F) / (x * PI_F);
}
