/*
 *     Copyright (C) 2025  Filippo Scognamiglio
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

#ifndef LIBRETRODROID_VIDEOLAYOUT_H
#define LIBRETRODROID_VIDEOLAYOUT_H

#include <array>

#include "utils/rect.h"

namespace libretrodroid {

class VideoLayout {
public:
    VideoLayout(float rotation, Rect viewportRect);

    void updateForegroundVertices();

    void updateAspectRatio(float aspectRatio);

    void updateScreenSize(unsigned screenWidth, unsigned screenHeight);

    void updateViewportSize(Rect viewportRect);

    void updateRotation(float rotation);

    std::array<float, 12>& getForegroundVertices();

    int getScreenWidth() { return screenWidth; }

    int getScreenHeight() { return screenHeight; }

    std::pair<float, float> getRelativePosition(float touchX, float touchY);


private:
    std::array<float, 12> videoVertices = {
        -1.0F,
        -1.0F,

        -1.0F,
        +1.0F,

        +1.0F,
        -1.0F,

        +1.0F,
        -1.0F,

        -1.0F,
        +1.0F,

        +1.0F,
        +1.0F,
    };

    float rotation = 0.0F;
    float aspectRatio = 1;
    Rect viewportRect = Rect(0.0F, 0.0F, 1.0F, 1.0F);

    unsigned screenWidth = 0;
    unsigned screenHeight = 0;
};

} // namespace libretrodroid

#endif //LIBRETRODROID_VIDEOLAYOUT_H
