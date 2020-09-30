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

oboe::DataCallbackResult LibretroDroid::Audio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    double framesCapacityInBuffer = fifo->getBufferCapacityInFrames();
    double framesAvailableInBuffer = fifo->getFullFramesAvailable();
    double framesToMid = framesCapacityInBuffer - 2.0f * framesAvailableInBuffer;

    double sampleRateAdjustment = 1 - 2 * MAX_AUDIO_ACCELERATION * framesToMid / framesCapacityInBuffer;
    double finalSampleRate = defaultSampleRate * sampleRateAdjustment;

    int32_t adjustedTotalFrames = numFrames * finalSampleRate;

    auto readFrames = fifo->readNow(audioBuffer.get(), adjustedTotalFrames * 2);
    if (readFrames != adjustedTotalFrames * 2) {
        LOGE("FILIPPO Buffer underrun detected");
    }

    auto outputArray = reinterpret_cast<int16_t *>(audioData);
    resampler.resample(audioBuffer.get(), adjustedTotalFrames, outputArray, numFrames);
    return oboe::DataCallbackResult::Continue;
}
