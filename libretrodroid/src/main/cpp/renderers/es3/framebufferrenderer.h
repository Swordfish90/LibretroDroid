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

#ifndef LIBRETRODROID_FRAMEBUFFERRENDERER_H
#define LIBRETRODROID_FRAMEBUFFERRENDERER_H

#include "GLES3/gl3.h"
#include "GLES3/gl3ext.h"

#include "../renderer.h"

namespace LibretroDroid {

class FramebufferRenderer: public LibretroDroid::Renderer {
public:
    FramebufferRenderer(unsigned width, unsigned height, bool depth, bool stencil, bool bilinearFiltering);
    uintptr_t getTexture() override;
    uintptr_t getFramebuffer() override;
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) override;
    void setPixelFormat(int pixelFormat) override;

private:
    unsigned int currentFramebuffer = 0;
    unsigned int currentTexture = 0;
};

}

#endif //LIBRETRODROID_FRAMEBUFFERRENDERER_H
