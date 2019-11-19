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

#ifndef HELLO_GL2_RENDERER_H
#define HELLO_GL2_RENDERER_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <cstdint>
#include <utility>

namespace LibretroDroid {

class Renderer {
public:
    virtual uintptr_t getFramebuffer() = 0;
    virtual uintptr_t getTexture() = 0;
    virtual void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);
    std::pair<int, int> lastFrameSize;
};

class ImageRenderer: public Renderer {
public:
    explicit ImageRenderer();
    uintptr_t getTexture();
    uintptr_t getFramebuffer();
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

private:
    GLuint currentTexture = 0;
};

class FramebufferRenderer: public Renderer {
public:
    FramebufferRenderer(unsigned width, unsigned height, bool depth, bool stencil);
    uintptr_t getTexture();
    uintptr_t getFramebuffer();
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

private:
    GLuint currentFramebuffer = 0;
    GLuint currentTexture = 0;
};

}


#endif //HELLO_GL2_RENDERER_H
