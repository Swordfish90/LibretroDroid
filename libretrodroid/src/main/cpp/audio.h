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

#include "resamplers/linearresampler.h"

namespace LibretroDroid {

class Audio: public oboe::AudioStreamCallback {
public:
    Audio(int32_t sampleRate);
    ~Audio() = default;

    void start();
    void stop();

    oboe::DataCallbackResult onAudioReady(oboe::AudioStream *oboeStream, void *audioData, int32_t numFrames) override;

    void write(const int16_t *data, size_t frames);
    void setSampleRateMultiplier(const double multiplier);

private:
    const double MAX_AUDIO_SPEED_PROPORTIONAL = 0.005;
    const double MAX_AUDIO_SPEED_INTEGRAL = 0.02;
    const int32_t AUDIO_LATENCY_MAX_MS = 60;

    static int32_t roundToEven(int32_t x);
    double computeAudioSpeedCoefficient(double dt);

    std::unique_ptr<oboe::FifoBuffer> fifo = nullptr;
    std::unique_ptr<int16_t[]> audioBuffer = nullptr;
    oboe::ManagedStream stream = nullptr;
    LinearResampler resampler;

    double defaultSampleRate;
    double errorMeasure = 0.0;
    double errorIntegral = 0.0;
    double sampleRateMultiplier = 1.0;
};

}

#endif //LIBRETRODROID_AUDIO_H
