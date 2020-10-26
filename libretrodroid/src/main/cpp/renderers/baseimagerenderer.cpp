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

#include "baseimagerenderer.h"

uintptr_t LibretroDroid::BaseImageRenderer::getTexture() {
    return currentTexture;
}

uintptr_t LibretroDroid::BaseImageRenderer::getFramebuffer() {
    return 0; // ImageRender does not really expose a framebuffer.
}

void LibretroDroid::BaseImageRenderer::setPixelFormat(int newPixelFormat) {
    pixelFormat = newPixelFormat;

    switch (newPixelFormat) {
        case RETRO_PIXEL_FORMAT_XRGB8888:
            this->glInternalFormat = GL_RGBA;
            this->glFormat = GL_RGBA;
            this->glType = GL_UNSIGNED_BYTE;
            this->bytesPerPixel = 4;
            this->swapRedAndBlueChannels = true;
            break;

        default:
        case RETRO_PIXEL_FORMAT_0RGB1555:
        case RETRO_PIXEL_FORMAT_RGB565:
            this->glInternalFormat = GL_RGB565;
            this->glFormat = GL_RGB;
            this->glType = GL_UNSIGNED_SHORT_5_6_5;
            this->bytesPerPixel = 2;
            this->swapRedAndBlueChannels = false;
            break;
    }
}

void
LibretroDroid::BaseImageRenderer::convertDataFrom0RGB1555(
        const void *data, unsigned int width, unsigned int height, size_t pitch) {
    auto castData = (uint16_t*) data;

    for (int i = 0; i < height * pitch / bytesPerPixel; ++i) {
        castData[i] = ((0x1Fu) & castData[i])
                      | (((0x1Fu << 5) & castData[i]) << 1)
                      | (((0x1Fu << 10) & castData[i]) << 1);
    }
}
