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

#include "videobackground.h"

#include <sstream>
#include <vector>

#include "log.h"

namespace libretrodroid {

void VideoBackground::initializeShaders() {
    if (blendShaderProgram != 0) return;

    GLuint blendingVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(blendingVertexShader, 1, &blendingVertexShaderSource, nullptr);
    glCompileShader(blendingVertexShader);

    GLuint blendingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(blendingFragmentShader, 1, &blendingFragmentShaderSource, nullptr);
    glCompileShader(blendingFragmentShader);

    blendShaderProgram = glCreateProgram();
    glAttachShader(blendShaderProgram, blendingVertexShader);
    glAttachShader(blendShaderProgram, blendingFragmentShader);
    glBindAttribLocation(blendShaderProgram, 0, "aPosition");
    glBindAttribLocation(blendShaderProgram, 1, "aTexCoord");
    glLinkProgram(blendShaderProgram);

    glDeleteShader(blendingVertexShader);
    glDeleteShader(blendingFragmentShader);

    blendTextureHandle = glGetUniformLocation(blendShaderProgram, "currentFrame");
    blendPrevTextureHandle = glGetUniformLocation(blendShaderProgram, "previousFrame");

    std::string fragmentShaderSource = generateBlurShader(7, 0.75);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &defaultVertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* c_str = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &c_str, nullptr);
    glCompileShader(fragmentShader);

    blurShaderProgram = glCreateProgram();
    glAttachShader(blurShaderProgram, vertexShader);
    glAttachShader(blurShaderProgram, fragmentShader);
    glBindAttribLocation(blurShaderProgram, 0, "aPosition");
    glBindAttribLocation(blurShaderProgram, 1, "aTexCoord");
    glLinkProgram(blurShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    blurPositionHandle = glGetAttribLocation(blurShaderProgram, "aPosition");
    blurTextureCoordinatesHandle = glGetAttribLocation(blurShaderProgram, "aTexCoord");
    blurTextureHandle = glGetUniformLocation(blurShaderProgram, "texture");
    blurDirectionHandle = glGetUniformLocation(blurShaderProgram, "direction");

    GLuint displayVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(displayVertexShader, 1, &defaultVertexShaderSource, nullptr);
    glCompileShader(displayVertexShader);

    GLuint displayFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(displayFragmentShader, 1, &displayFragmentShaderSource, nullptr);
    glCompileShader(displayFragmentShader);

    displayShaderProgram = glCreateProgram();
    glAttachShader(displayShaderProgram, displayVertexShader);
    glAttachShader(displayShaderProgram, displayFragmentShader);
    glBindAttribLocation(displayShaderProgram, 0, "aPosition");
    glBindAttribLocation(displayShaderProgram, 1, "aTexCoord");
    glLinkProgram(displayShaderProgram);

    glDeleteShader(displayVertexShader);
    glDeleteShader(displayFragmentShader);

    displayPositionHandle = glGetAttribLocation(blurShaderProgram, "aPosition");
    displayTextureCoordinatesHandle = glGetAttribLocation(blurShaderProgram, "aTexCoord");
    displayTextureHandle = glGetUniformLocation(displayShaderProgram, "texture");
}

void VideoBackground::initializeFramebuffers() {
    if (!blurFramebuffers.empty()) return;

    for (int i = 0; i < 4; i++) {
        blurFramebuffers.push_back(
            ES3Utils::createFramebuffer(downscaledWidth, downscaledHeight, true, false, false)
        );
    }
}

void VideoBackground::renderToFramebuffer(uintptr_t texture, GLfloat* gBackgroundVertices) {
    int blendFramebufferReadIndex = (blendFramebufferWriteIndex + 1) % 2;

    glViewport(0, 0, downscaledWidth, downscaledHeight);

    glVertexAttribPointer(blurPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gBackgroundVertices);
    glEnableVertexAttribArray(blurPositionHandle);

    glVertexAttribPointer(blurTextureCoordinatesHandle, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);
    glEnableVertexAttribArray(blurTextureCoordinatesHandle);

    glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffers[blendFramebufferWriteIndex]->framebuffer);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blendShaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(blendTextureHandle, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, blurFramebuffers[blendFramebufferReadIndex]->texture);
    glUniform1i(blendPrevTextureHandle, 1);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(blurShaderProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffers[2]->framebuffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurFramebuffers[blendFramebufferWriteIndex]->texture);
    glUniform1i(blurTextureHandle, 0);
    glUniform2f(blurDirectionHandle, 1.0 / downscaledWidth, 0.0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, blurFramebuffers[3]->framebuffer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurFramebuffers[2]->texture);
    glUniform1i(blurTextureHandle, 0);
    glUniform2f(blurDirectionHandle, 0.0, 1.0 / downscaledHeight);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    blendFramebufferWriteIndex = blendFramebufferReadIndex;
}

void VideoBackground::renderToFinalOutput(
    unsigned screenWidth,
    unsigned screenHeight,
    std::array<float, 12> backgroundVertices,
    std::array<float, 12> foregroundVertices
) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glUseProgram(displayShaderProgram);

    glVertexAttribPointer(displayPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, backgroundVertices.data());
    glEnableVertexAttribArray(displayPositionHandle);

    glVertexAttribPointer(displayTextureCoordinatesHandle, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);
    glEnableVertexAttribArray(displayTextureCoordinatesHandle);

    glUniform1i(displayTextureHandle, 0);

    float foregroundMinX = +1.0F;
    float foregroundMinY = +1.0F;
    float foregroundMaxX = -1.0F;
    float foregroundMaxY = -1.0F;

    for (size_t i = 0; i < foregroundVertices.size(); i += 2) {
        float x = foregroundVertices[i];
        float y = foregroundVertices[i + 1];

        foregroundMinX = std::min(foregroundMinX, x);
        foregroundMinY = std::min(foregroundMinY, -y);
        foregroundMaxX = std::max(foregroundMaxX, x);
        foregroundMaxY = std::max(foregroundMaxY, -y);
    }

    // Send it to the shader
    GLint foregroundLoc = glGetUniformLocation(displayShaderProgram, "uForegroundBounds");
    glUniform4f(foregroundLoc, foregroundMinX, foregroundMinY, foregroundMaxX, foregroundMaxY);

// Set shadow properties
    GLint shadowStrengthLoc = glGetUniformLocation(displayShaderProgram, "uShadowStrength");
    glUniform1f(shadowStrengthLoc, 0.5f);  // 0.5 = 50% opacity

    GLint shadowSpreadLoc = glGetUniformLocation(displayShaderProgram, "uShadowSpread");
    glUniform1f(shadowSpreadLoc, 0.05f);  // Controls softness

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurFramebuffers[3]->texture);
    glUniform1i(displayTextureHandle, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(displayPositionHandle);
    glDisableVertexAttribArray(displayTextureCoordinatesHandle);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void VideoBackground::renderBackground(
    unsigned screenWidth,
    unsigned screenHeight,
    std::array<float, 12> backgroundVertices,
    std::array<float, 12> foregroundVertices,
    GLfloat* framebufferVertices,
    uintptr_t texture
) {
    initializeShaders();
    initializeFramebuffers();

    if (blendFramebufferCurrent == 0) {
        renderToFramebuffer(texture, framebufferVertices);
    }
    // TODO BLEND... Decide what to do with this...
    blendFramebufferCurrent = (blendFramebufferCurrent + 1) % 2;

    renderToFinalOutput(screenWidth, screenHeight, backgroundVertices, foregroundVertices);
}

std::vector<float> VideoBackground::generateSmoothingWeights(int size, float brightness) {
    std::vector<float> kernel(size);
    float sigma = size / 3.0f;
    float sum = 0.0f;
    int halfSize = size / 2;

    for (int i = 0; i < size; i++) {
        int x = i - halfSize;
        kernel[i] = exp(-(x * x) / (2 * sigma * sigma));
        sum += kernel[i];
    }

    for (int i = 0; i < size; i++) {
        kernel[i] = (kernel[i] / sum) * brightness;
    }

    return kernel;
}

std::string VideoBackground::generateBlurShader(int maskSize, float brightness) {
    if (maskSize % 2 == 0) {
        LOGE("Error: maskSize should be an odd number!");
        return "";
    }

    std::vector<float> kernel = generateSmoothingWeights(maskSize, brightness);

    std::ostringstream result;
    result << R"(
        precision mediump float;
        varying vec2 vTexCoord;
        uniform sampler2D texture;
        uniform vec2 direction;

        void main() {
            lowp vec4 result = vec4(0.0);
            lowp float kernel[)" << maskSize << R"(];

    )";

    for (int i = 0; i < maskSize; i++) {
        result << "            kernel[" << i << "] = " << kernel[i] << ";\n";
    }

    int halfMask = maskSize / 2;
    result << R"(
            for (int i = -)" << halfMask << "; i <= " << halfMask << R"(; i++) {
                vec2 offset = vec2(float(i)) * direction;
                result += texture2D(texture, vTexCoord + offset) * kernel[i + )" << halfMask << R"(];
            }
            gl_FragColor = result;
        }
    )";

    return result.str();
}

} // namespace libretrodroid