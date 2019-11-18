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