//
// Created by swordfish on 10/11/19.
//

#ifndef HELLO_GL2_INPUT_H
#define HELLO_GL2_INPUT_H

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

#endif //HELLO_GL2_INPUT_H
