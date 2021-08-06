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

#ifndef LIBRETRODROID_RUMBLE_H
#define LIBRETRODROID_RUMBLE_H

#include <jni.h>

namespace libretrodroid {

// TODO FILIPPO... This should be enabled again with the new implementation...
class Rumble {
public:
    void setEnabled(bool enabled);
    void update(uint16_t currentStrength);
    bool hasUpdate();
    float getCurrentValue();

private:
    bool enabled = false;
    bool dirty = false;
    uint16_t currentRumbleStrength = 0;
};

}


#endif //LIBRETRODROID_RUMBLE_H
