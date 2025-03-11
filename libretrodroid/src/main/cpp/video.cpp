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
#include "renderers/es3/framebufferrenderer.h"
#include "renderers/es3/imagerendereres3.h"
#include "renderers/es2/imagerendereres2.h"

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

    loadedShaderType = requestedShaderConfig;

    auto shaders = ShaderManager::getShader(requestedShaderConfig);

    shadersChain = {};

    std::for_each(shaders.passes.begin(), shaders.passes.end(), [&](const auto& item){
        auto shader = ShaderChainEntry { };

        shader.gProgram = createProgram(item.vertex.data(), item.fragment.data());
        if (!shader.gProgram) {
            LOGE("Could not create gl program.");
            throw std::runtime_error("Cannot create gl program");
        }

        shader.gvPositionHandle = glGetAttribLocation(shader.gProgram, "vPosition");

        shader.gvCoordinateHandle = glGetAttribLocation(shader.gProgram, "vCoordinate");

        shader.gTextureHandle = glGetUniformLocation(shader.gProgram, "texture");

        shader.gPreviousPassTextureHandle = glGetUniformLocation(shader.gProgram, "previousPass");

        shader.gTextureSizeHandle = glGetUniformLocation(shader.gProgram, "textureSize");

        shader.gScreenDensityHandle = glGetUniformLocation(shader.gProgram, "screenDensity");

        shader.gvFlipYHandle = glGetUniformLocation(shader.gProgram, "vFlipY");

        shader.gViewModelMatrixHandle = glGetUniformLocation(shader.gProgram, "vViewModel");

        shadersChain.push_back(shader);
    });

    renderer->setShaders(shaders);
}

// TODO BLUR... We should probably rotate also the background...
void Video::updateForegroundVertices() {
    LOGD("Computing foreground vertices from screen (%d x %d), aspect ratio (%f) with rotation (%f radians)", screenWidth, screenHeight, aspectRatio, rotation);

    float screenW = screenWidth  * viewportRect.getWidth();
    float screenH = screenHeight * viewportRect.getHeight();
    float screenAspect  = screenW / screenH;
    float contentAspect = aspectRatio;

    float scaleX = viewportRect.getWidth();
    float scaleY = viewportRect.getHeight();
    if (contentAspect > screenAspect) {
        scaleY *= screenAspect / contentAspect;
    } else {
        scaleX *= contentAspect / screenAspect;
    }
    // At this point, without rotation the quad covers:
    // x in [-scaleX, scaleX] and y in [-scaleY, scaleY].

    float viewportXOffset = (viewportRect.getX() * 2.0f) - (1.0f - viewportRect.getWidth());
    float viewportYOffset = (viewportRect.getY() * 2.0f) - (1.0f - viewportRect.getHeight());

    float cosTheta = cos(-rotation);
    float sinTheta = sin(-rotation);

    float factorX = scaleX / (scaleX * fabs(cosTheta) + scaleY * fabs(sinTheta));
    float factorY = scaleY / (scaleX * fabs(sinTheta) + scaleY * fabs(cosTheta));

    float uv[4][2] = {
        { -1.0f, -1.0f },
        { -1.0f, +1.0f },
        { +1.0f, -1.0f },
        { +1.0f, +1.0f }
    };

    float rotatedQuad[4][2];
    for (int i = 0; i < 4; i++) {
        float u = uv[i][0];
        float v = uv[i][1];

        float origX = u * scaleX;
        float origY = v * scaleY;

        float rawX = origX * cosTheta - origY * sinTheta;
        float rawY = origX * sinTheta + origY * cosTheta;

        float finalX = rawX * factorX + viewportXOffset;
        float finalY = rawY * factorY - viewportYOffset;

        rotatedQuad[i][0] = finalX;
        rotatedQuad[i][1] = finalY;
    }

    gForegroundVertices[0]  = rotatedQuad[0][0];
    gForegroundVertices[1]  = rotatedQuad[0][1];

    gForegroundVertices[2]  = rotatedQuad[1][0];
    gForegroundVertices[3]  = rotatedQuad[1][1];

    gForegroundVertices[4]  = rotatedQuad[2][0];
    gForegroundVertices[5]  = rotatedQuad[2][1];

    gForegroundVertices[6]  = rotatedQuad[2][0];
    gForegroundVertices[7]  = rotatedQuad[2][1];

    gForegroundVertices[8]  = rotatedQuad[1][0];
    gForegroundVertices[9]  = rotatedQuad[1][1];

    gForegroundVertices[10] = rotatedQuad[3][0];
    gForegroundVertices[11] = rotatedQuad[3][1];
}

void Video::renderFrame() {
    if (skipDuplicateFrames && !isDirty) return;
    isDirty = false;

    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderBackground();

    updateProgram();
    for (int i = 0; i < shadersChain.size(); ++i) {
        auto shader = shadersChain[i];
        auto passData = renderer->getPassData(i);
        auto isLastPass = i == shadersChain.size() - 1;

        glBindFramebuffer(GL_FRAMEBUFFER, passData.framebuffer.value_or(0));

        glViewport(0, 0, passData.width.value_or(screenWidth), passData.height.value_or(screenHeight));

        glUseProgram(shader.gProgram);

        auto vertices = isLastPass ? gForegroundVertices : gBackgroundVertices;
        glVertexAttribPointer(shader.gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(shader.gvPositionHandle);

        glVertexAttribPointer(shader.gvCoordinateHandle, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);
        glEnableVertexAttribArray(shader.gvCoordinateHandle);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderer->getTexture());
        glUniform1i(shader.gTextureHandle, 0);

        if (shader.gPreviousPassTextureHandle != -1 && passData.texture.has_value()) {
            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_2D, passData.texture.value());
            glUniform1i(shader.gPreviousPassTextureHandle, 1);
        }

        glUniform2f(shader.gTextureSizeHandle, getTextureWidth(), getTextureHeight());

        glUniform1f(shader.gvFlipYHandle, gFlipY);

        glUniform1f(shader.gScreenDensityHandle, getScreenDensity());

        glUniformMatrix4fv(shader.gViewModelMatrixHandle, 1, false, gViewModelMatrix);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(shader.gvPositionHandle);
        glDisableVertexAttribArray(shader.gvCoordinateHandle);

        glBindTexture(GL_TEXTURE_2D, 0);

        glUseProgram(0);
    }
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

// TODO BLUR. At this point the viewmodel matrix can be completely removed.
void Video::updateViewModelMatrix(float rotation) {
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
    updateForegroundVertices();
}

void Video::updateViewportSize(Rect viewportRect) {
    this->viewportRect = viewportRect;
    updateForegroundVertices();
}

void Video::updateRendererSize(unsigned int width, unsigned int height) {
    LOGD("Updating renderer size: %d x %d", width, height);
    renderer->updateRenderedResolution(width, height);
}

void Video::updateRotation(float rotation) {
//    updateViewModelMatrix(rotation);
}

Video::Video(
    RenderingOptions renderingOptions,
    ShaderManager::Config shaderConfig,
    bool bottomLeftOrigin,
    float rotation,
    bool skipDuplicateFrames,
    Rect viewportRect
) :
    viewportRect(viewportRect),
    requestedShaderConfig(std::move(shaderConfig)),
    skipDuplicateFrames(skipDuplicateFrames),
    gFlipY(bottomLeftOrigin ? 0.0F : 1.0F),
    rotation(rotation) {

    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);
    initializeGLESLogCallbackIfNeeded();

    LOGI("Initializing graphics");

//    updateViewModelMatrix(rotation);

    updateForegroundVertices();

    glViewport(0, 0, screenWidth, screenHeight);

    glUseProgram(0);

    initializeRenderer(renderingOptions);
}

void Video::updateShaderType(ShaderManager::Config shaderConfig) {
    requestedShaderConfig = std::move(shaderConfig);
}

void Video::initializeRenderer(RenderingOptions renderingOptions) {
    auto shaders = ShaderManager::getShader(requestedShaderConfig);

    if (renderingOptions.hardwareAccelerated) {
        renderer = new FramebufferRenderer(
            renderingOptions.width,
            renderingOptions.height,
            renderingOptions.useDepth,
            renderingOptions.useStencil,
            std::move(shaders)
        );
    } else {
        if (renderingOptions.openglESVersion >= 3) {
            renderer = new ImageRendererES3();
        } else {
            renderer = new ImageRendererES2();
        }
    }

    renderer->setPixelFormat(renderingOptions.pixelFormat);
    updateProgram();
}

void Video::renderBackground() {
    static GLuint blurShaderProgram = 0;
    static GLuint fbo1 = 0, fbo2 = 0;
    static GLuint texture1 = 0, texture2 = 0;
    static GLint positionHandle = -1;
    static GLint texCoordHandle = -1;
    static GLint textureHandle = -1;
    static GLint directionHandle = -1;

    static GLuint displayShaderProgram = 0;
    static GLint displayTextureHandle = -1;
    static GLint displayFlipYHandle = -1;

    static int smallWidth = 8;
    static int smallHeight = 8;

    if (blurShaderProgram == 0) {
        const char* vertexShaderSource = R"(
            attribute vec2 aPosition;
            attribute vec2 aTexCoord;
            uniform mediump float vFlipY;

            varying vec2 vTexCoord;
            void main() {
                gl_Position = vec4(aPosition, 0.0, 1.0);
                vTexCoord = vec2(aTexCoord.x, mix(aTexCoord.y, 1.0 - aTexCoord.y, vFlipY));
            }
        )";

        const char* fragmentShaderSource = R"(
            precision mediump float;
            varying vec2 vTexCoord;
            uniform sampler2D texture;
            uniform vec2 direction;

            void main() {
                lowp vec4 sum = vec4(0.0);
                float kernel[5];
                kernel[0] = 0.227027;
                kernel[1] = 0.1945946;
                kernel[2] = 0.1216216;
                kernel[3] = 0.054054;
                kernel[4] = 0.016216;
                for (int i = -2; i <= 2; i++) {
                    vec2 offset = vec2(float(i)) * direction;
                    sum += texture2D(texture, vTexCoord + offset) * kernel[i + 2];
                }
                gl_FragColor = sum;
            }
        )";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        blurShaderProgram = glCreateProgram();
        glAttachShader(blurShaderProgram, vertexShader);
        glAttachShader(blurShaderProgram, fragmentShader);
        glBindAttribLocation(blurShaderProgram, 0, "aPosition");
        glBindAttribLocation(blurShaderProgram, 1, "aTexCoord");
        glLinkProgram(blurShaderProgram);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        positionHandle = glGetAttribLocation(blurShaderProgram, "aPosition");
        texCoordHandle = glGetAttribLocation(blurShaderProgram, "aTexCoord");
        textureHandle = glGetUniformLocation(blurShaderProgram, "texture");
        directionHandle = glGetUniformLocation(blurShaderProgram, "direction");

        glGenFramebuffers(1, &fbo1);
        glGenFramebuffers(1, &fbo2);
        glGenTextures(1, &texture1);
        glGenTextures(1, &texture2);

        GLuint textures[] = { texture1, texture2 };
        for (unsigned int texture : textures) {
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, smallWidth, smallHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture1, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2, 0);

        const char* displayVertexShaderSource = R"(
            attribute vec2 aPosition;
            attribute vec2 aTexCoord;
            uniform mediump float vFlipY;
            varying vec2 vTexCoord;
            void main() {
                gl_Position = vec4(aPosition, 0.0, 1.0);
                vTexCoord = vec2(aTexCoord.x, mix(aTexCoord.y, 1.0 - aTexCoord.y, vFlipY));
            }
        )";

        const char* displayFragmentShaderSource = R"(
            precision mediump float;
            varying vec2 vTexCoord;
            uniform mediump float vFlipY;
            uniform sampler2D texture;
            void main() {
                gl_FragColor = texture2D(texture, vTexCoord);
            }
        )";

        GLuint displayVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(displayVertexShader, 1, &displayVertexShaderSource, nullptr);
        glCompileShader(displayVertexShader);

        GLuint displayFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(displayFragmentShader, 1, &displayFragmentShaderSource, nullptr);
        glCompileShader(displayFragmentShader);


        displayShaderProgram = glCreateProgram();
        glAttachShader(displayShaderProgram, displayVertexShader);
        glAttachShader(displayShaderProgram, displayFragmentShader);
        glBindAttribLocation(displayShaderProgram, 0, "aPosition");
        glBindAttribLocation(displayShaderProgram, 1, "aTexCoord");
        displayFlipYHandle = glGetUniformLocation(blurShaderProgram, "vFlipY");
        glLinkProgram(displayShaderProgram);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, fbo1);
    glViewport(0, 0, smallWidth, smallHeight);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blurShaderProgram);

    glVertexAttribPointer(positionHandle, 2, GL_FLOAT, GL_FALSE, 0, gBackgroundVertices);
    glEnableVertexAttribArray(positionHandle);

    glVertexAttribPointer(texCoordHandle, 2, GL_FLOAT, GL_FALSE, 0, gTextureCoords);
    glEnableVertexAttribArray(texCoordHandle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->getTexture());
    glUniform1i(textureHandle, 0);
    glUniform2f(directionHandle, 1.0 / smallWidth, 0.0);

    // TODO FILIPPO... Handle gFlipY
//    glUniform1f(flipYHandle, gFlipY);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glUniform2f(directionHandle, 0.0, 1.0 / smallHeight);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, screenWidth, screenHeight);
    glUseProgram(displayShaderProgram);
    glUniform1i(displayTextureHandle, 0);
    glUniform1f(displayFlipYHandle, gFlipY);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionHandle);
    glDisableVertexAttribArray(texCoordHandle);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
}

void Video::updateAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
    updateForegroundVertices();
}

} //namespace libretrodroid
