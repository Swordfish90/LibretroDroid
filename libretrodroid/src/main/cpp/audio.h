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

namespace libretrodroid {

class Audio: public oboe::AudioStreamDataCallback, oboe::AudioStreamErrorCallback {
private:
    struct AudioLatencySettings {
        double kp;
        double ki;
        double maxp;
        double maxi;
        bool useLowLatencyStream;
    };

    const AudioLatencySettings PI_SETTINGS_STANDARD {
        0.006,
        0.00002,
        0.003,
        0.02,
        false
    };

    const AudioLatencySettings PI_SETTINGS_LOW_32 {
        0.006,
        0.00002,
        0.003,
        0.02,
        true
    };

public:
    Audio(int32_t sampleRate, double refreshRate, bool preferLowLatencyAudio);
    ~Audio() = default;

    void start();
    void stop();

    oboe::DataCallbackResult onAudioReady(
        oboe::AudioStream *oboeStream,
        void *audioData,
        int32_t numFrames
    ) override;

    void onErrorAfterClose(oboe::AudioStream *oldStream, oboe::Result result) override;

public:
    void write(const int16_t *data, size_t frames);
    void setPlaybackSpeed(const double newPlaybackSpeed);

private:
    static int32_t roundToEven(int32_t x);
    double computeDynamicBufferConversionFactor(double dt);
    int32_t computeAudioBufferSize();
    bool initializeStream();
    std::unique_ptr<Audio::AudioLatencySettings> findBestLatencySettings(bool preferLowLatencyAudio);
    double computeMaximumLatency() const;

private:
    LinearResampler resampler;
    std::unique_ptr<oboe::FifoBuffer> fifoBuffer = nullptr;
    std::unique_ptr<int16_t[]> temporaryAudioBuffer = nullptr;

    oboe::ManagedStream stream = nullptr;
    std::unique_ptr<oboe::LatencyTuner> latencyTuner = nullptr;

    bool startRequested = false;
    int32_t inputSampleRate;
    double contentRefreshRate = 60.0;

    double baseConversionFactor = 1.0;

    double framesToSubmit = 0.0;
    double errorIntegral = 0.0;

    double playbackSpeed = 1.0;

    std::unique_ptr<AudioLatencySettings> audioLatencySettings;
};

} // namespace libretrodroid

#endif //LIBRETRODROID_AUDIO_H
