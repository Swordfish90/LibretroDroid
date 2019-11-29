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

#include "input.h"

#include <cmath>

#include <android/input.h>
#include <android/keycodes.h>

#include "libretro/libretro.h"

int16_t LibretroDroid::Input::getInputState(unsigned port, unsigned device, unsigned index, unsigned id) {
    // TODO... We need to handle multiple input source for local multiplayer...
    if (port != 0) {
        return 0;
    }

    switch (device) {
        case RETRO_DEVICE_JOYPAD:
            switch (id) {
                case RETRO_DEVICE_ID_JOYPAD_LEFT:
                    return dpadXAxis == -1;
                case RETRO_DEVICE_ID_JOYPAD_RIGHT:
                    return dpadXAxis == 1;
                case RETRO_DEVICE_ID_JOYPAD_UP:
                    return dpadYAxis == -1;
                case RETRO_DEVICE_ID_JOYPAD_DOWN:
                    return dpadYAxis == 1;
                default:
                    return pressedKeys.count(id) > 0;
            }

        case RETRO_DEVICE_ANALOG:
            switch (index) {
                case RETRO_DEVICE_INDEX_ANALOG_LEFT:
                    switch (id) {
                        case RETRO_DEVICE_ID_ANALOG_X:
                            return (int16_t) (joypadLeftXAxis * ANALOG_MAX_RANGE);
                        case RETRO_DEVICE_ID_ANALOG_Y:
                            return (int16_t) (joypadLeftYAxis * ANALOG_MAX_RANGE);
                        default:
                            return 0;
                    }
                case RETRO_DEVICE_INDEX_ANALOG_RIGHT:
                    switch (id) {
                        case RETRO_DEVICE_ID_ANALOG_X:
                            return (int16_t) (joypadRightXAxis * ANALOG_MAX_RANGE);
                        case RETRO_DEVICE_ID_ANALOG_Y:
                            return (int16_t) (joypadRightYAxis * ANALOG_MAX_RANGE);
                        default:
                            return 0;
                    }
                default:
                    return 0;
            }

        default:
            return 0;
    }
}

int LibretroDroid::Input::convertAndroidToLibretroKey(int keyCode) {
    switch (keyCode) {
        case AKEYCODE_BUTTON_START:
            return RETRO_DEVICE_ID_JOYPAD_START;
        case AKEYCODE_BUTTON_SELECT:
            return RETRO_DEVICE_ID_JOYPAD_SELECT;
        case AKEYCODE_BUTTON_A:
            return RETRO_DEVICE_ID_JOYPAD_B;
        case AKEYCODE_BUTTON_X:
            return RETRO_DEVICE_ID_JOYPAD_Y;
        case AKEYCODE_BUTTON_Y:
            return RETRO_DEVICE_ID_JOYPAD_X;
        case AKEYCODE_BUTTON_B:
            return RETRO_DEVICE_ID_JOYPAD_A;
        case AKEYCODE_BUTTON_L1:
            return RETRO_DEVICE_ID_JOYPAD_L;
        case AKEYCODE_BUTTON_L2:
            return RETRO_DEVICE_ID_JOYPAD_L2;
        case AKEYCODE_BUTTON_R1:
            return RETRO_DEVICE_ID_JOYPAD_R;
        case AKEYCODE_BUTTON_R2:
            return RETRO_DEVICE_ID_JOYPAD_R2;
    }
    return UNKNOWN_KEY;
}

bool LibretroDroid::Input::onKeyEvent(int action, int keyCode) {
    int retroKeyCode = convertAndroidToLibretroKey(keyCode);
    if (retroKeyCode == UNKNOWN_KEY) {
        return false;
    }

    if (action == AKEY_EVENT_ACTION_DOWN) {
        pressedKeys.insert(retroKeyCode);
    } else if (action == AKEY_EVENT_ACTION_UP) {
        pressedKeys.erase(retroKeyCode);
    }
    return true;
}

bool LibretroDroid::Input::onMotionEvent(int motionSource, float xAxis, float yAxis) {
    switch (motionSource) {
        case LibretroDroid::Input::MOTION_SOURCE_DPAD:
            dpadXAxis = (int) round(xAxis);
            dpadYAxis = (int) round(yAxis);
            break;

        case LibretroDroid::Input::MOTION_SOURCE_ANALOG_LEFT:
            joypadLeftXAxis = xAxis;
            joypadLeftYAxis = yAxis;
            break;

        case LibretroDroid::Input::MOTION_SOURCE_ANALOG_RIGHT:
            joypadRightXAxis = xAxis;
            joypadRightYAxis = yAxis;
            break;

        default:
            return false;
    }

    return true;
}
