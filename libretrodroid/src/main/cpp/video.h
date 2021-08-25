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

#include <GLES2/gl2.h>

#include "renderers/renderer.h"

namespace libretrodroid {

class Video {
public:
    void initializeGraphics(Renderer* renderer, const std::string& fragmentShader, bool bottomLeftOrigin, float rotation);
    void updateScreenSize(unsigned screenWidth, unsigned screenHeight);
    void updateRendererSize(unsigned width, unsigned height);
    void updateRotation(float rotation);

    void renderFrame();

    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

    uintptr_t getCurrentFramebuffer() {
        return renderer->getFramebuffer();
    };

private:
    void updateViewModelMatrix();
    float getScreenDensity();
    float getTextureWidth();
    float getTextureHeight();

private:
    GLfloat gTriangleVertices[12] = {
            -1.0F,
            -1.0F,

            -1.0F,
             1.0F,

             1.0F,
            -1.0F,

             1.0F,
            -1.0F,

            -1.0F,
             1.0F,

             1.0F,
             1.0F,
    };

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

    GLfloat gViewModelMatrix[16] = {
        1.0F, 0.0F, 0.0F, 0.0F,
        0.0F, 1.0F, 0.0F, 0.0F,
        0.0F, 0.0F, 1.0F, 0.0F,
        0.0F, 0.0F, 0.0F, 1.0F,
    };

    unsigned screenWidth = 0;
    unsigned screenHeight = 0;

    float rotation = 0;

    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint gTextureHandle = 0;
    GLint gScreenDensityHandle = 0;
    GLint gTextureSizeHandle = 0;
    GLint gFlipYHandle = 0;
    GLint gViewModelMatrixHandle = 0;

    float gFlipY = 1.0;

    Renderer* renderer;
};

}

#endif //LIBRETRODROID_VIDEO_H
