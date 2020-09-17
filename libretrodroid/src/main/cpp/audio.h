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

#ifndef LIBRETRODROID_AUDIO_H
#define LIBRETRODROID_AUDIO_H

#include <unistd.h>
#include <oboe/Oboe.h>
#include "oboe/src/fifo/FifoBuffer.h"

namespace LibretroDroid {

class Audio: public oboe::AudioStreamCallback {
public:
    Audio(int32_t sampleRate);
    ~Audio() = default;

    void start();
    void stop();

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

    void write(const int16_t *data, size_t frames);

private:
    std::unique_ptr<oboe::FifoBuffer> fifo = nullptr;
    oboe::ManagedStream stream = nullptr;
};

}

#endif //LIBRETRODROID_AUDIO_H
