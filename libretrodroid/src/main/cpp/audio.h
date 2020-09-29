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

#include <array>
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

    static float sinc(float x);
    void resample_sinc(const int16_t* source, int32_t inputFrames, int16_t* sink, int32_t sinkFrames);
    void resample_linear(const int16_t* source, int32_t inputFrames, int16_t* sink, int32_t sinkFrames);

    void write(const int16_t *data, size_t frames);

private:
    static constexpr float PI_F = 3.14159265358979f;
    static constexpr float MAX_AUDIO_ACCELERATION = 0.02;
    static constexpr int SINC_RESAMPLING_TAPS = 2;

    std::unique_ptr<oboe::FifoBuffer> fifo = nullptr;
    std::unique_ptr<int16_t[]> audioBuffer = nullptr;
    oboe::ManagedStream stream = nullptr;

    double defaultSampleRate;
};

}

#endif //LIBRETRODROID_AUDIO_H
