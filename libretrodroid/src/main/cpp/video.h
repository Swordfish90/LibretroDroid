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

#ifndef LIBRETRODROID_VIDEO_H
#define LIBRETRODROID_VIDEO_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "renderer.h"

namespace LibretroDroid {

class Video {
public:
    void initializeGraphics(Renderer* renderer, const std::string& fragmentShader, bool bottomLeftOrigin);
    void updateScreenSize(int screenWidth, int screenHeight);

    void renderFrame();

    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

    uintptr_t getCurrentFramebuffer() {
        return renderer->getFramebuffer();
    };

private:
    void updateVertices();
    float getScreenDensity();
    float getTextureWidth();
    float getTextureHeight();

private:
    GLfloat gTriangleVertices[12];
    GLfloat gTriangleCoords[12] = {
        0.0F,
        0.0F,

        0.0F,
        1.0F,

        1.0F,
        0.0F,

        1.0F,
        0.0F,

        0.0F,
        1.0F,

        1.0F,
        1.0F,
    };

    int screenWidth = 0;
    int screenHeight = 0;

    float finalScreenWidth = 0;
    float finalScreenHeight = 0;

    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint gTextureHandle = 0;
    GLint gScreenDensityHandle = 0;
    GLint gTextureSizeHandle = 0;
    GLint gFlipYHandle = 0;

    float gFlipY = 1.0;

    Renderer* renderer;
};

}

#endif //LIBRETRODROID_VIDEO_H
