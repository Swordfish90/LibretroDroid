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
