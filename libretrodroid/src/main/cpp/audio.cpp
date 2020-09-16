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

void LibretroDroid::Audio::initializeAudio(int32_t sampleRate) {
    LOGI("Audio initialization has been called with sample rate %d", sampleRate);

    // We are buffering a max of 125ms of audio.
    circularBuffer = CircularBuffer(sampleRate / 2);

    oboe::AudioStreamBuilder builder;
    builder.setChannelCount(2);
    builder.setSampleRate(sampleRate);
    builder.setDirection(oboe::Direction::Output);
    builder.setFormat(oboe::AudioFormat::I16);
    builder.setCallback(this);

    oboe::Result result = builder.openManagedStream(stream);
    if (result != oboe::Result::OK) {
        LOGE("Failed to create stream. Error: %s", oboe::convertToText(result));
    }
}

LibretroDroid::Audio::Audio(int32_t sampleRate) {
    initializeAudio(sampleRate);
    stream->open();
}

void LibretroDroid::Audio::start() {
    stream->requestStart();
}

void LibretroDroid::Audio::stop() {
    stream->requestStop();
}

void LibretroDroid::Audio::write(const int16_t *data, size_t frames) {
    size_t size = frames * 2 * sizeof(int16_t);
    LOGV("Audio buffer write: %d bytes", size);
    circularBuffer.write((const unsigned char *) data, size);
}

oboe::DataCallbackResult LibretroDroid::Audio::onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) {
    size_t size = numFrames * 2 * sizeof(int16_t);

    if (circularBuffer.usedSize() < 0.2 * circularBuffer.capacity()) {
        LOGV("Audio buffer is too empty. Skipping to fill it.");
        memset(audioData, 0, size);
        return oboe::DataCallbackResult::Continue;
    }

    if (circularBuffer.availableSize() < 0.2 * circularBuffer.capacity()) {
        LOGV("Audio buffer is too full. Dropping oldest samples.");
        circularBuffer.drop(size);
    }

    circularBuffer.read((unsigned char*) audioData, size);

    LOGV("Audio buffer read of %d bytes", size);

    return oboe::DataCallbackResult::Continue;
}
