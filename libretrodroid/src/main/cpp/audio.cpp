//
// Created by swordfish on 18/10/19.
//

#include "log.h"

#include "audio.h"

void LibretroDroid::Audio::initializeAudio(int32_t sampleRate) {
    LOGI("Audio initialization has been called with sample rate %d", sampleRate);

    oboe::AudioStreamBuilder builder;
    builder.setChannelCount(2);
    builder.setSampleRate(sampleRate);
    builder.setDirection(oboe::Direction::Output);
    builder.setFormat(oboe::AudioFormat::I16);

    oboe::Result result = builder.openStream(&stream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
    }
}

LibretroDroid::Audio::Audio(int32_t sampleRate) {
    initializeAudio(sampleRate);
}

LibretroDroid::Audio::~Audio() {
    stream->close();
}

void LibretroDroid::Audio::start() {
    stream->start(oboe::kDefaultTimeoutNanos);
}

void LibretroDroid::Audio::stop() {
    stream->stop(oboe::kDefaultTimeoutNanos);
}

void LibretroDroid::Audio::write(const int16_t *data, size_t frames) {
    stream->write(data, frames, oboe::kDefaultTimeoutNanos);
}
