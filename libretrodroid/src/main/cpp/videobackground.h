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
#ifndef LIBRETRODROID_VIDEOBACKGROUND_H
#define LIBRETRODROID_VIDEOBACKGROUND_H

#include <string>
#include <GLES2/gl2.h>

#include "renderers/es3/es3utils.h"

namespace libretrodroid {

class VideoBackground {
public:
    void renderBackground(
        unsigned screenWidth,
        unsigned screenHeight,
        GLfloat* backgroundVertices,
        GLfloat* framebufferVertices,
        uintptr_t texture
    );

private:
    static std::string generateBlurShader(int maskSize, float brightness);
    static std::vector<float> generateSmoothingWeights(int size, float brightness);
    void initializeShaders();
    void initializeFramebuffers();
    void renderToFramebuffer(uintptr_t texture, GLfloat* gBackgroundVertices);
    void renderToFinalOutput(
        unsigned int screenWidth,
        unsigned int screenHeight,
        GLfloat* gBackgroundVertices
    );

private:
    const char* defaultVertexShaderSource = R"(
        attribute mediump vec2 aPosition;
        attribute mediump vec2 aTexCoord;
        varying mediump vec2 vTexCoord;
        void main() {
            gl_Position = vec4(aPosition, 0.0, 1.0);
            vTexCoord = aTexCoord;
        }
    )";

    const char* displayFragmentShaderSource = R"(
        precision mediump float;
        varying mediump vec2 vTexCoord;
        uniform lowp sampler2D texture;
        void main() {
            gl_FragColor = texture2D(texture, vTexCoord);
        }
    )";

    const char* blendingVertexShaderSource = R"(
        attribute mediump vec2 aPosition;
        attribute mediump vec2 aTexCoord;
        varying mediump vec2 vTexCoord;
        void main() {
            gl_Position = vec4(aPosition, 0.0, 1.0);
            vTexCoord = aTexCoord;
        }
    )";

    const char* blendingFragmentShaderSource = R"(
        precision mediump float;
        varying mediump vec2 vTexCoord;
        uniform lowp sampler2D currentFrame;
        uniform lowp sampler2D previousFrame;

        void main() {
            lowp vec4 currentColor = texture2D(currentFrame, vTexCoord);
            lowp vec4 prevColor = texture2D(previousFrame, vTexCoord);
            gl_FragColor = mix(prevColor, currentColor, 0.1);
        }
    )";

    GLfloat gTextureCoords[12] = {
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

    GLuint blurShaderProgram = 0;
    std::vector<std::unique_ptr<ES3Utils::Framebuffer>> blurFramebuffers;

    GLint blurPositionHandle = -1;
    GLint blurTextureCoordinatesHandle = -1;
    GLint blurTextureHandle = -1;

    GLint blurDirectionHandle = -1;

    GLuint displayShaderProgram = 0;
    GLint displayTextureHandle = -1;
    GLint displayPositionHandle = -1;
    GLint displayTextureCoordinatesHandle = -1;

    int downscaledWidth = 8;
    int downscaledHeight = 8;

    GLuint blendShaderProgram = 0;
    GLint blendTextureHandle = -1;
    GLint blendPrevTextureHandle = -1;
    int blendFramebufferWriteIndex = 0;
    int blendFramebufferCurrent = 0;
};

} // libretrodroid

#endif //LIBRETRODROID_VIDEOBACKGROUND_H
