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

#ifndef HELLO_GL2_VIDEO_H
#define HELLO_GL2_VIDEO_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "renderer.h"

namespace LibretroDroid {

class Video {
public:
    void initializeGraphics(Renderer* renderer, bool bottomLeftOrigin, float aspectRatio);
    void updateScreenSize(int screenWidth, int screenHeight);

    void renderFrame();

    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

    uintptr_t getCurrentFramebuffer() {
        return renderer->getFramebuffer();
    };

private:
    void updateCoords(unsigned width, unsigned height, size_t pitch);

    void updateVertices();

private:
    GLfloat gTriangleVertices[12];
    GLfloat gTriangleCoords[12];

    bool bottomLeftOrigin = false;
    int screenWidth = 0;
    int screenHeight = 0;
    float aspectRatio = 1.0F;

    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint textureHandle = 0;

    Renderer* renderer;
};

}

#endif //HELLO_GL2_VIDEO_H
