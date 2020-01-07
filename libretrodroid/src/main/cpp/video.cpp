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

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <cstdlib>
#include <string>

#include "log.h"

#include "video.h"

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGE("after %s() glError (0x%x)\n", op, error);
    }
}

const char* gVertexShader =
        "attribute vec4 vPosition;\n"
        "attribute vec2 vCoordinate;\n"
        "uniform mediump float vPadding;\n"
        "uniform mediump float vFlipY;\n"
        "uniform lowp sampler2D texture;\n"
        "varying vec2 coords;"
        "varying vec2 origCoords;\n"
        "void main() {\n"
        "  origCoords = vCoordinate;\n"
        "  coords.x = vCoordinate.x * vPadding;\n"
        "  coords.y = mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY);\n"
        "  gl_Position = vPosition;\n"
        "}\n";

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
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
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

void LibretroDroid::Video::initializeGraphics(Renderer* renderer, const std::string& fragmentShader, bool bottomLeftOrigin, float aspectRatio) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    this->renderer = renderer;
    this->aspectRatio = aspectRatio;
    this->bottomLeftOrigin = bottomLeftOrigin;

    LOGI("Initializing graphics");

    gProgram = createProgram(gVertexShader, fragmentShader.data());
    if (!gProgram) {
        LOGE("Could not create gl program.");
        throw std::runtime_error("Cannot create gl program");
    }

    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");

    gvCoordinateHandle = glGetAttribLocation(gProgram, "vCoordinate");
    checkGlError("glGetAttribLocation");

    gTextureHandle = glGetUniformLocation(gProgram, "texture");
    checkGlError("glGetAttribLocation");

    gTextureSizeHandle = glGetUniformLocation(gProgram, "textureSize");
    checkGlError("glGetAttribLocation");

    gFlipYHandle = glGetUniformLocation(gProgram, "vFlipY");
    checkGlError("glGetAttribLocation");

    gPaddingHandle = glGetUniformLocation(gProgram, "vPadding");
    checkGlError("glGetAttribLocation");

    glViewport(0, 0, screenWidth, screenHeight);
    checkGlError("glViewport");

    glUseProgram(0);
}

void LibretroDroid::Video::renderFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glViewport(0, 0, screenWidth, screenHeight);

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    glDisable(GL_DEPTH_TEST);

    updateVertices();
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");

    glVertexAttribPointer(gvCoordinateHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleCoords);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvCoordinateHandle);
    checkGlError("glEnableVertexAttribArray");

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, renderer->getTexture());
    checkGlError("glBindTexture");

    glUniform1i(gTextureHandle, 0);
    checkGlError("glUniform1i");

    glUniform2f(gTextureSizeHandle, renderer->lastFrameSize.first, renderer->lastFrameSize.second);
    checkGlError("glUniform2f");

    glUniform1f(gPaddingHandle, gPadding);
    checkGlError("glUniform1f");

    glUniform1f(gFlipYHandle, gFlipY);
    checkGlError("glUniform1f");

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGlError("glDrawArrays");

    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("glBindTexture");

    glUseProgram(0);
}

void LibretroDroid::Video::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    renderer->onNewFrame(data, width, height, pitch);
    updateCoords(width, height, pitch);
}

void LibretroDroid::Video::updateCoords(unsigned width, unsigned height, size_t pitch) {
    if (pitch >= width) {
        gPadding = (float) renderer->getBytesPerPixel() * width / pitch;
    } else {
        gPadding = 1.0F;
    }

    gFlipY = bottomLeftOrigin ? 0 : 1;

    gTriangleCoords[0] = 0.0F;
    gTriangleCoords[1] = 0.0F;

    gTriangleCoords[2] = 0.0F;
    gTriangleCoords[3] = 1.0F;

    gTriangleCoords[4] = 1.0F;
    gTriangleCoords[5] = 0.0F;

    gTriangleCoords[6] = 1.0F;
    gTriangleCoords[7] = 0.0F;

    gTriangleCoords[8] = 0.0F;
    gTriangleCoords[9] = 1.0F;

    gTriangleCoords[10] = 1.0F;
    gTriangleCoords[11] = 1.0F;
}

void LibretroDroid::Video::updateVertices() {
    float screenAspectRatio = (float) screenWidth / screenHeight;

    float scaleX = 1.0F;
    float scaleY = 1.0F;

    if (aspectRatio > screenAspectRatio) {
        scaleY = screenAspectRatio / aspectRatio;
    } else {
        scaleX = aspectRatio / screenAspectRatio;
    }

    LOGD("Updating vertices position with %f %f %f %f", scaleX, scaleY, screenAspectRatio, aspectRatio);

    gTriangleVertices[0] = -1.0F * scaleX;
    gTriangleVertices[1] = -1.0F * scaleY;

    gTriangleVertices[2] = -1.0F * scaleX;
    gTriangleVertices[3] = 1.0F * scaleY;

    gTriangleVertices[4] = 1.0F * scaleX;
    gTriangleVertices[5] = -1.0F * scaleY;

    gTriangleVertices[6] = 1.0F * scaleX;
    gTriangleVertices[7] = -1.0F * scaleY;

    gTriangleVertices[8] = -1.0F * scaleX;
    gTriangleVertices[9] = 1.0F * scaleY;

    gTriangleVertices[10] = 1.0F * scaleX;
    gTriangleVertices[11] = 1.0F * scaleY;
}

void LibretroDroid::Video::updateScreenSize(int screenWidth, int screenHeight) {
    LOGD("Updating screen size: %d x %d", screenWidth, screenHeight);
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}
