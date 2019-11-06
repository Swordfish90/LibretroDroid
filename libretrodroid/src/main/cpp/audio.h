//
// Created by swordfish on 18/10/19.
//

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
    void pause();

    void write(const int16_t *data, size_t frames);

private:
    void initializeAudio(int32_t sampleRate);

    oboe::AudioStream* stream = nullptr;
};

}

#endif //HELLO_GL2_AUDIO_H
