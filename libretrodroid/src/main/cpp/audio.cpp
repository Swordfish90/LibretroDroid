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

namespace libretrodroid {

Audio::Audio(int32_t sampleRate, int requestedLatencyMode) {
    LOGI("Audio initialization has been called with input sample rate %d", sampleRate);

    inputSampleRate = sampleRate;
    audioLatencySettings = findBestLatencySettings(requestedLatencyMode);
    initializeStream();
}

bool Audio::initializeStream() {
    LOGI("Using low latency stream: %d", audioLatencySettings->useLowLatencyStream);

    oboe::AudioStreamBuilder builder;
    builder.setChannelCount(2);
    builder.setDirection(oboe::Direction::Output);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setDataCallback(this);
    builder.setErrorCallback(this);

    if (audioLatencySettings->useLowLatencyStream) {
        builder.setPerformanceMode(oboe::PerformanceMode::LowLatency);
    }

    int32_t audioBufferSize = computeAudioBufferSize();

    oboe::Result result = builder.openManagedStream(stream);
    if (result == oboe::Result::OK) {
        baseConversionFactor = (double) inputSampleRate / stream->getSampleRate();
        fifoBuffer = std::make_unique<oboe::FifoBuffer>(2, audioBufferSize);
        temporaryAudioBuffer = std::unique_ptr<int16_t[]>(new int16_t[audioBufferSize]);
        latencyTuner = std::make_unique<oboe::LatencyTuner>(*stream);
        return true;
    } else {
        LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
        stream = nullptr;
        latencyTuner = nullptr;
        return false;
    }
}

std::unique_ptr<Audio::AudioLatencySettings> Audio::findBestLatencySettings(int latencyMode) {
    if (!oboe::AudioStreamBuilder::isAAudioRecommended()) {
        return std::make_unique<AudioLatencySettings>(PI_SETTINGS_STANDARD);
    }

    switch (latencyMode) {
        case AUDIO_LATENCY_MODE_LOW_64:
            return std::make_unique<AudioLatencySettings>(PI_SETTINGS_LOW_64);
        case AUDIO_LATENCY_MODE_LOW_32:
            return std::make_unique<AudioLatencySettings>(PI_SETTINGS_LOW_32);
        default:
            return std::make_unique<AudioLatencySettings>(PI_SETTINGS_STANDARD);
    }
}

int32_t Audio::computeAudioBufferSize() {
    double sampleRateDivisor = 500.0 / audioLatencySettings->maxLatencyMs;
    return roundToEven(inputSampleRate / sampleRateDivisor);
}

void Audio::start() {
    startRequested = true;
    if (stream != nullptr)
        stream->requestStart();
}

void Audio::stop() {
    startRequested = false;
    if (stream != nullptr)
        stream->requestStop();
}

void Audio::write(const int16_t *data, size_t frames) {
    fifoBuffer->write(data, frames * 2);
}

void Audio::setPlaybackSpeed(const double newPlaybackSpeed) {
    playbackSpeed = newPlaybackSpeed;
}

oboe::DataCallbackResult Audio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    double dynamicBufferFactor = computeDynamicBufferConversionFactor(0.001 * numFrames);
    double finalConversionFactor = baseConversionFactor * dynamicBufferFactor * playbackSpeed;

    int32_t adjustedTotalFrames = std::round(numFrames * finalConversionFactor);

    fifoBuffer->readNow(temporaryAudioBuffer.get(), adjustedTotalFrames * 2);

    auto outputArray = reinterpret_cast<int16_t *>(audioData);
    resampler.resample(temporaryAudioBuffer.get(), adjustedTotalFrames, outputArray, numFrames);

    latencyTuner->tune();

    return oboe::DataCallbackResult::Continue;
}

// To prevent audio buffer overruns or underruns we set up a PI controller. The idea is to run the
// audio slower when the buffer is empty and faster when it's full.
double Audio::computeDynamicBufferConversionFactor(double dt) {
    double framesCapacityInBuffer = fifoBuffer->getBufferCapacityInFrames();
    double framesAvailableInBuffer = fifoBuffer->getFullFramesAvailable();

    // Error is represented by normalized distance to half buffer utilization. Range [-1.0, 1.0]
    double errorMeasure = (framesCapacityInBuffer - 2.0f * framesAvailableInBuffer) / framesCapacityInBuffer;

    errorIntegral += errorMeasure * dt;

    // Wikipedia states that human ear resolution is around 3.6 Hz within the octave of 1000–2000 Hz.
    // This changes continuously, so we should try to keep it a very low value.
    double proportionalAdjustment = std::clamp(
            audioLatencySettings->kp * errorMeasure,
            -audioLatencySettings->maxp,
            audioLatencySettings->maxp
    );

    // Ki is a lot lower, so it's safe if it exceeds the ear threshold. Hopefully convergence will
    // be slow enough to be not perceptible. We need to battle test this value.
    double integralAdjustment = std::clamp(
            audioLatencySettings->ki * errorIntegral,
            -audioLatencySettings->maxi,
            audioLatencySettings->maxi
    );

    double finalAdjustment = std::clamp(
            proportionalAdjustment + integralAdjustment,
            -audioLatencySettings->finalMax,
            audioLatencySettings->finalMax
    );

    LOGD("Spped adjustments (p: %f) (i: %f) (%f)", proportionalAdjustment, integralAdjustment);

    return 1.0 - (finalAdjustment);
}

int32_t Audio::roundToEven(int32_t x) {
    return (x / 2) * 2;
}

void Audio::onErrorAfterClose(oboe::AudioStream* oldStream, oboe::Result result) {
    AudioStreamErrorCallback::onErrorAfterClose(oldStream, result);
    LOGI("Stream error in oboe::onErrorAfterClose %s", oboe::convertToText(result));

    if (result != oboe::Result::ErrorDisconnected)
        return;

    initializeStream();
    if (startRequested) {
        start();
    }
}

} //namespace libretrodroid
