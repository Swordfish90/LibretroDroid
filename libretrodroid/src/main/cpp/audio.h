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

#ifndef HELLO_GL2_AUDIO_H
#define HELLO_GL2_AUDIO_H

#include <unistd.h>
#include <oboe/Oboe.h>

namespace LibretroDroid {

class Audio {
public:
    Audio(int32_t sampleRate);
    ~Audio();

    void start();
    void stop();

    void write(const int16_t *data, size_t frames);

private:
    void initializeAudio(int32_t sampleRate);

    oboe::AudioStream* stream = nullptr;
};

}

#endif //HELLO_GL2_AUDIO_H
