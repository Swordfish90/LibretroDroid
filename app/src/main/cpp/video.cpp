//
// Created by swordfish on 18/10/19.
//

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
    for (GLint error = glGetError(); error; error
                                                    = glGetError()) {
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

void LibretroDroid::Video::initializeGraphics(int screenWidth, int screenHeight, bool bottomLeftOrigin) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    this->bottomLeftOrigin = bottomLeftOrigin;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    LOGI("setupGraphics(%d, %d)", screenWidth, screenHeight);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        exit(2);
    }

    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n", gvPositionHandle);

    gvCoordinateHandle = glGetAttribLocation(gProgram, "vCoordinate");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vCoordinate\") = %d\n", gvCoordinateHandle);

    textureHandle = glGetUniformLocation(gProgram, "texture");
    checkGlError("glGetAttribLocation");
    LOGI("glGetUniformLocation(\"texture\") = %d\n", textureHandle);

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

    if (use3DRendering) {
        glActiveTexture(GL_TEXTURE0);
        checkGlError("glActiveTexture");
        glBindTexture(GL_TEXTURE_2D, current_framebuffer_texture);
        checkGlError("glBindTexture");
        glUniform1i(textureHandle, 0);
        checkGlError("glUniform1i");
    } else {
        glActiveTexture(GL_TEXTURE0);
        checkGlError("glActiveTexture");
        glBindTexture(GL_TEXTURE_2D, current_texture);
        checkGlError("glBindTexture");
        glUniform1i(textureHandle, 0);
        checkGlError("glUniform1i");
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGlError("glDrawArrays");

    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("glBindTexture");

    glUseProgram(0);
}

void LibretroDroid::Video::initialize3DRendering(int width, int height, float aspectRatio, bool depth, bool stencil) {
    this->use3DRendering = true;
    this->aspectRatio = aspectRatio;

    glGenFramebuffers(1, &current_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, current_framebuffer);

    glGenTextures(1, &current_framebuffer_texture);
    glBindTexture(GL_TEXTURE_2D, current_framebuffer_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, current_framebuffer_texture, 0);

    if (depth) {
        unsigned depth_buffer;
        glGenRenderbuffers(1, &depth_buffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer);
        glRenderbufferStorage(GL_RENDERBUFFER, stencil ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT16, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, stencil? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_buffer);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("Error while create framebuffer. Leaving!");
        exit(2);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void LibretroDroid::Video::initialize2DRendering(int width, int height, float aspectRatio) {
    this->use3DRendering = false;
    this->aspectRatio = aspectRatio;

    glGenTextures(1, &current_texture);
    checkGlError("glGenTextures");
    glBindTexture(GL_TEXTURE_2D, current_texture);
    checkGlError("glBindTexture");
}

void LibretroDroid::Video::onNew2DFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    LOGI("Received new frame of size (%d, %d) %d", width, height, pitch);
    glBindTexture(GL_TEXTURE_2D, current_texture);
    checkGlError("glBindTexture");
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, pitch / 2, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
    checkGlError("glTexImage2D");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    updateCoords(width, height, pitch);
}

void LibretroDroid::Video::onNew3DFrame(const void *data, unsigned width, unsigned height, size_t pitch) {

}

void LibretroDroid::Video::updateCoords(unsigned width, unsigned height, size_t pitch) {
    float padding = (float) 2 * width / pitch;
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

    LOGD("Correcting vertices position with %f %f %f %f", scaleX, scaleY, screenAspectRatio, aspectRatio);

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
