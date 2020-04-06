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

#include "imagerendereres3.h"
#include "../../libretro/libretro.h"

LibretroDroid::ImageRendererES3::ImageRendererES3() {
    glGenTextures(1, &currentTexture);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
}

void LibretroDroid::ImageRendererES3::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    glBindTexture(GL_TEXTURE_2D, currentTexture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, bytesPerPixel);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pitch / bytesPerPixel);

    if (swapRedAndBlueChannels) {
        applyGLSwizzle(GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA);
    } else {
        applyGLSwizzle(GL_RED, GL_GREEN, GL_BLUE, GL_ALPHA);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (lastFrameSize.first != width || lastFrameSize.second != height) {
        glTexImage2D(GL_TEXTURE_2D, 0, glInternalFormat, width, height, 0, glFormat, glType, data);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, glFormat, glType, data);
    }

    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    LibretroDroid::Renderer::onNewFrame(data, width, height, pitch);
}

void LibretroDroid::ImageRendererES3::applyGLSwizzle(int r, int g, int b, int a) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, r);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, g);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, b);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, a);
}

uintptr_t LibretroDroid::ImageRendererES3::getTexture() {
    return currentTexture;
}

uintptr_t LibretroDroid::ImageRendererES3::getFramebuffer() {
    return 0; // ImageRender does not really expose a framebuffer.
}

void LibretroDroid::ImageRendererES3::setPixelFormat(int pixelFormat) {
    switch (pixelFormat) {

        case RETRO_PIXEL_FORMAT_XRGB8888:
            this->glInternalFormat = GL_RGBA;
            this->glFormat = GL_RGBA;
            this->glType = GL_UNSIGNED_BYTE;
            this->bytesPerPixel = 4;
            this->swapRedAndBlueChannels = true;
            break;

        default:
        case RETRO_PIXEL_FORMAT_RGB565:
            this->glInternalFormat = GL_RGB565;
            this->glFormat = GL_RGB;
            this->glType = GL_UNSIGNED_SHORT_5_6_5;
            this->bytesPerPixel = 2;
            this->swapRedAndBlueChannels = false;
            break;
    }
}