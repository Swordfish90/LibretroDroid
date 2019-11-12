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
