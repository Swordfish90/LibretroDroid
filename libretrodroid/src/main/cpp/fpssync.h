//
// Created by swordfish on 05/11/19.
//

#ifndef HELLO_GL2_FPSSYNC_H
#define HELLO_GL2_FPSSYNC_H

#include <chrono>
#include <thread>

namespace LibretroDroid {

class FPSSync {
public:
    FPSSync(double framerate);
    ~FPSSync() { }

    void sync();
    void start();

private:
    double framerate;
    bool useVSync;

    std::chrono::time_point<std::chrono::steady_clock> lastFrame;
    std::chrono::duration<long, std::micro> sampleInterval;
};

}


#endif //HELLO_GL2_FPSSYNC_H
