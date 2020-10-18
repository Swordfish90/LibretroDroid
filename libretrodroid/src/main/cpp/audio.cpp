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
#include <memory>

LibretroDroid::Audio::Audio(int32_t sampleRate) {
    LOGI("Audio initialization has been called with sample rate %d", sampleRate);

    int32_t sampleRateDivisor = 500 / AUDIO_LATENCY_MAX_MS;
    int32_t sampleRateBufferSize = sampleRate / sampleRateDivisor;

    fifo = std::make_unique<oboe::FifoBuffer>(2, roundToEven(sampleRateBufferSize));
    audioBuffer = std::unique_ptr<int16_t[]>(new int16_t[sampleRateBufferSize]);

    oboe::AudioStreamBuilder builder;
    builder.setChannelCount(2);
    builder.setDirection(oboe::Direction::Output);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setCallback(this);
    builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);

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
    fifo->write(data, size);
}

void LibretroDroid::Audio::setSampleRateMultiplier(const double multiplier) {
    sampleRateMultiplier = multiplier;
}

oboe::DataCallbackResult LibretroDroid::Audio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    double finalSampleRate = defaultSampleRate *
            computeAudioSpeedCoefficient(0.001 * numFrames) *
            sampleRateMultiplier;

    int32_t adjustedTotalFrames = numFrames * finalSampleRate;

    fifo->readNow(audioBuffer.get(), adjustedTotalFrames * 2);

    auto outputArray = reinterpret_cast<int16_t *>(audioData);
    resampler.resample(audioBuffer.get(), adjustedTotalFrames, outputArray, numFrames);
    return oboe::DataCallbackResult::Continue;
}

// To prevent audio buffer overruns or underruns we set up a PI controller. The idea is to run the
// audio slower when the buffer is empty and faster when it's full.
double LibretroDroid::Audio::computeAudioSpeedCoefficient(double dt) {
    double framesCapacityInBuffer = fifo->getBufferCapacityInFrames();
    double framesAvailableInBuffer = fifo->getFullFramesAvailable();

    // Error is represented by normalized distance to half buffer utilization. Range [-1.0, 1.0]
    double error = (framesCapacityInBuffer - 2.0f * framesAvailableInBuffer) / framesCapacityInBuffer;

    // Low-pass filter to error measure since it's very noisy.
    errorMeasure = errorMeasure * 0.8 + error * 0.2;

    errorIntegral += errorMeasure * dt;

    // Wikipedia states that human ear resolution is around 3.6 Hz within the octave of 1000–2000 Hz.
    // This changes continuously, so we should try to keep it a very low value.
    double proportionalAdjustment = std::clamp(0.005 * errorMeasure, -MAX_AUDIO_SPEED_PROPORTIONAL, MAX_AUDIO_SPEED_PROPORTIONAL);

    // Ki is a lot lower, so it's safe if it exceeds the ear threshold. Hopefully convergence will
    // be slow enough to be not perceptible. We need to battle test this value.
    double integralAdjustment = std::clamp(0.000005 * errorIntegral, -MAX_AUDIO_SPEED_INTEGRAL, MAX_AUDIO_SPEED_INTEGRAL);

    return 1.0 - (proportionalAdjustment + integralAdjustment);
}

int32_t LibretroDroid::Audio::roundToEven(int32_t x) {
    return (x / 2) * 2;
}
