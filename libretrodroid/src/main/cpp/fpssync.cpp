//
// Created by swordfish on 05/11/19.
//

#include <cmath>
#include "fpssync.h"
#include "log.h"

void LibretroDroid::FPSSync::sync() {
    if (!useVSync) {
        std::this_thread::sleep_until(lastFrame + sampleInterval);
        lastFrame = lastFrame + sampleInterval;
    }
}

LibretroDroid::FPSSync::FPSSync(double framerate) {
    // We rely on vsync if framerate is close to 60fps.
    this->framerate = framerate;
    useVSync = std::fabs(60.0 - framerate) < 1.0;
    sampleInterval = std::chrono::microseconds((long) ((1000000L / framerate)));
}

void LibretroDroid::FPSSync::start() {
    LOGI("Starting game with fps: %f. Using vsync: %d", framerate, !useVSync);

    lastFrame = lastFrame = std::chrono::steady_clock::now();
}
