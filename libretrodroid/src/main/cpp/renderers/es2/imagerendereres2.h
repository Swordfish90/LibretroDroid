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

#ifndef LIBRETRODROID_IMAGERENDERERES2_H
#define LIBRETRODROID_IMAGERENDERERES2_H

#include "GLES2/gl2.h"

#include "../renderer.h"
#include "../../libretro/libretro.h"
#include "../baseimagerenderer.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace LibretroDroid {

class ImageRendererES2: public LibretroDroid::BaseImageRenderer {

public:
    explicit ImageRendererES2();
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) override;

private:
    void convertDataFromRGB8888(const void* pixelData, size_t size);
};

}

#endif //LIBRETRODROID_IMAGERENDERERES2_H
