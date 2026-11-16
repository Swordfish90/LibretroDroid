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

#include <stdexcept>

#include "framebufferrenderer.h"
#include "es3utils.h"
#include "../../sharedcontext.h"
#include "../../log.h"

namespace libretrodroid {

FramebufferRenderer::FramebufferRenderer(
    unsigned width,
    unsigned height,
    bool depth,
    bool stencil,
    ShaderManager::Chain shaders,
    SharedContext* sharedContext
) : sharedContext(sharedContext) {
    this->depth = depth;
    this->stencil = stencil;
    this->width = width;
    this->height = height;
    this->shaders = std::move(shaders);

    initializeBuffers();
}

void FramebufferRenderer::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    Renderer::onNewFrame(data, width, height, pitch);

    if (isDirty) {
        initializeBuffers();
        isDirty = false;
    }
}

void FramebufferRenderer::initializeBuffers() {
    framebuffers = ES3Utils::buildShaderPasses(width, height, shaders);

    ES3Utils::deleteFramebuffer(std::move(framebuffer));
    framebuffer = ES3Utils::createFramebuffer(
        width,
        height,
        shaders.linearTexture,
        false,
        depth,
        stencil
    );

    if (sharedContext) {
        sharedContext->bindCore();
        GLint previousFramebuffer;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousFramebuffer);
        if (!coreFramebuffer) glGenFramebuffers(1, &coreFramebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, coreFramebuffer);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->texture, 0);
        glFramebufferRenderbuffer(
            GL_DRAW_FRAMEBUFFER,
            stencil ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER,
            framebuffer->depth.value_or(0)
        );

        auto status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousFramebuffer);
        sharedContext->bindFrontend();
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error("Cannot create the core framebuffer");
        }
    }
}

uintptr_t FramebufferRenderer::getTexture() {
    return framebuffer->texture;
}

uintptr_t FramebufferRenderer::getFramebuffer() {
    return sharedContext ? coreFramebuffer : framebuffer->framebuffer;
}

void FramebufferRenderer::setPixelFormat(int pixelFormat) {
    // TODO... Here we should handle 32bit framebuffers.
}

void FramebufferRenderer::updateRenderedResolution(unsigned int width, unsigned int height) {
    if (this->width != width || this->height != height) {
        this->width = width;
        this->height = height;
        isDirty = true;
    }
}

void FramebufferRenderer::setShaders(ShaderManager::Chain shaders) {
    if (shaders != this->shaders) {
        this->shaders = shaders;
        isDirty = true;
    }
}

Renderer::PassData FramebufferRenderer::getPassData(unsigned int layer) {
    PassData result;

    if (layer >= 0 && layer < framebuffers->size()) {
        result.framebuffer = framebuffers->at(layer)->framebuffer;
        result.width = framebuffers->at(layer)->width;
        result.height = framebuffers->at(layer)->height;
    }

    if (layer > 0 && layer < framebuffers->size() + 1) {
        result.texture = framebuffers->at(layer - 1)->texture;
    }

    return result;
}

} //namespace libretrodroid
