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

#ifndef LIBRETRODROID_INPUT_H
#define LIBRETRODROID_INPUT_H

#include <cstdint>
#include <unordered_set>

namespace LibretroDroid {

class Input {

public:
    static const int MOTION_SOURCE_DPAD = 0;
    static const int MOTION_SOURCE_ANALOG_LEFT = 1;
    static const int MOTION_SOURCE_ANALOG_RIGHT = 2;
    static const int ANALOG_MAX_RANGE = 0x7fff;

    int16_t getInputState(unsigned port, unsigned device, unsigned index, unsigned id);

    bool onKeyEvent(int action, int keyCode);
    bool onMotionEvent(int motionSource, float xAxis, float yAxis);

private:
    const int UNKNOWN_KEY = -1;

    int convertAndroidToLibretroKey(int keyCode);

    std::unordered_set<int> pressedKeys;

    int dpadXAxis = 0;
    int dpadYAxis = 0;
    float joypadLeftXAxis = 0;
    float joypadLeftYAxis = 0;
    float joypadRightXAxis = 0;
    float joypadRightYAxis = 0;
};

}

#endif //LIBRETRODROID_INPUT_H
