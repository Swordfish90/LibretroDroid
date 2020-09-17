/*
 *     Copyright (C) 2020  Filippo Scognamiglio
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

#include "rumble.h"

void LibretroDroid::Rumble::updateAndDispatch(uint16_t currentStrength, JNIEnv* env, jobject glRetroView) {
    if (!enabled || currentStrength == currentRumbleStrength) return;

    currentRumbleStrength = currentStrength;

    if (rumbleMethodId == nullptr) {
        jclass cls = env->GetObjectClass(glRetroView);
        rumbleMethodId = env->GetMethodID(cls, "sendRumbleStrength", "(F)V");
    }

    float finalVibration = (float) currentRumbleStrength / 0xFFFF;
    env->CallVoidMethod(glRetroView, rumbleMethodId, finalVibration);
}

void LibretroDroid::Rumble::setEnabled(bool enabled) {
    this->enabled = enabled;
}
