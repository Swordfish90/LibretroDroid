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
        "uniform lowp sampler2D texture;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "  coords = vCoordinate;\n"
        "  gl_Position = vPosition;\n"
        "}\n";

const char* gFragmentShader =
        "precision mediump float;\n"
        "uniform lowp sampler2D texture;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "  vec4 tex = texture2D(texture, coords);"
        "  gl_FragColor = vec4(tex.rgb, 1.0);\n"
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

void LibretroDroid::Video::initializeGraphics(Renderer* renderer, bool bottomLeftOrigin, float aspectRatio) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    this->renderer = renderer;
    this->aspectRatio = aspectRatio;
    this->bottomLeftOrigin = bottomLeftOrigin;

    LOGI("Initializing graphics");

    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        exit(2);
    }

    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");

    gvCoordinateHandle = glGetAttribLocation(gProgram, "vCoordinate");
    checkGlError("glGetAttribLocation");

    textureHandle = glGetUniformLocation(gProgram, "texture");
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
    checkGlError("glEnableVertexAttribArray");;

    glActiveTexture(GL_TEXTURE0);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, renderer->getTexture());
    checkGlError("glBindTexture");
    glUniform1i(textureHandle, 0);
    checkGlError("glUniform1i");

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
    float padding = pitch >= width ? (float) 2 * width / pitch : 1.0F;
    int flipY = bottomLeftOrigin ? 1 : -1;

    gTriangleCoords[0] = 0.0F * padding;
    gTriangleCoords[1] = 0.0F * flipY;

    gTriangleCoords[2] = 0.0F * padding;
    gTriangleCoords[3] = 1.0F * flipY;

    gTriangleCoords[4] = 1.0F * padding;
    gTriangleCoords[5] = 0.0F * flipY;

    gTriangleCoords[6] = 1.0F * padding;
    gTriangleCoords[7] = 0.0F * flipY;

    gTriangleCoords[8] = 0.0F * padding;
    gTriangleCoords[9] = 1.0F * flipY;

    gTriangleCoords[10] = 1.0F * padding;
    gTriangleCoords[11] = 1.0F * flipY;
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
    LOGI("Updating screen size: %d x %d", screenWidth, screenHeight);
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}
