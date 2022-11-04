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

#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <cstdlib>
#include <string>
#include <cmath>
#include <utility>

#include "log.h"

#include "video.h"

namespace libretrodroid {

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, nullptr);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, nullptr, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, nullptr, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

void Video::updateProgram() {
    if (loadedShaderType.has_value() && loadedShaderType.value() == requestedShaderConfig) {
        return;
    }

    auto shaders = ShaderManager::getShader(requestedShaderConfig);

    gProgram = createProgram(shaders.vertex.data(), shaders.fragment.data());
    if (!gProgram) {
        LOGE("Could not create gl program.");
        throw std::runtime_error("Cannot create gl program");
    }

    renderer->setLinear(shaders.linear);

    loadedShaderType = requestedShaderConfig;

    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");

    gvCoordinateHandle = glGetAttribLocation(gProgram, "vCoordinate");

    gTextureHandle = glGetUniformLocation(gProgram, "texture");

    gTextureSizeHandle = glGetUniformLocation(gProgram, "textureSize");

    gScreenDensityHandle = glGetUniformLocation(gProgram, "screenDensity");

    gFlipYHandle = glGetUniformLocation(gProgram, "vFlipY");

    gViewModelMatrixHandle = glGetUniformLocation(gProgram, "vViewModel");
}

void Video::renderFrame() {
    updateProgram();

    if (skipDuplicateFrames && !isDirty) return;
    isDirty = false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, screenWidth, screenHeight);

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgram);

    glDisable(GL_DEPTH_TEST);

    updateViewModelMatrix();

    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    glEnableVertexAttribArray(gvPositionHandle);

    glVertexAttribPointer(gvCoordinateHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleCoords);
    glEnableVertexAttribArray(gvCoordinateHandle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->getTexture());

    glUniform1i(gTextureHandle, 0);

    glUniform2f(gTextureSizeHandle, getTextureWidth(), getTextureHeight());

    glUniform1f(gScreenDensityHandle, getScreenDensity());

    glUniform1f(gFlipYHandle, gFlipY);

    glUniformMatrix4fv(gViewModelMatrixHandle, 1, false, gViewModelMatrix);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(gvPositionHandle);
    glDisableVertexAttribArray(gvCoordinateHandle);

    glBindTexture(GL_TEXTURE_2D, 0);

    glUseProgram(0);
}

float Video::getScreenDensity() {
    return std::min(screenWidth / getTextureWidth(), screenHeight / getTextureHeight());
}

float Video::getTextureWidth() {
    return renderer->lastFrameSize.first;
}

float Video::getTextureHeight() {
    return renderer->lastFrameSize.second;
}

void Video::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    if (data != nullptr) {
        renderer->onNewFrame(data, width, height, pitch);
        isDirty = true;
    }
}

void Video::updateViewModelMatrix() {
    // Apply simple rotation matrix
    gViewModelMatrix[0] = cos(rotation);
    gViewModelMatrix[1] = -sin(rotation);
    gViewModelMatrix[4] = sin(rotation);
    gViewModelMatrix[5] = cos(rotation);
}

void Video::updateScreenSize(unsigned screenWidth, unsigned screenHeight) {
    LOGD("Updating screen size: %d x %d", screenWidth, screenHeight);
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}

void Video::updateRendererSize(unsigned int width, unsigned int height) {
    LOGD("Updating renderer size: %d x %d", width, height);
    renderer->updateRenderedResolution(width, height);
}

void Video::updateRotation(float rotation) {
    this->rotation = rotation;
}

Video::Video(
    Renderer* renderer,
    ShaderManager::Config shaderConfig,
    bool bottomLeftOrigin,
    float rotation,
    bool skipDuplicateFrames
) :
    requestedShaderConfig(std::move(shaderConfig)),
    rotation(rotation),
    skipDuplicateFrames(skipDuplicateFrames),
    gFlipY(bottomLeftOrigin ? 0 : 1),
    renderer(renderer) {

    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    initializeGLESLogCallbackIfNeeded();

    LOGI("Initializing graphics");

    glViewport(0, 0, screenWidth, screenHeight);

    glUseProgram(0);
}

void Video::updateShaderType(ShaderManager::Config shaderConfig) {
    requestedShaderConfig = std::move(shaderConfig);
}
} //namespace libretrodroid
