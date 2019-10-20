//
// Created by swordfish on 20/10/19.
//

#include <cstdlib>
#include "log.h"

#include "renderer.h"

LibretroDroid::ImageRenderer::ImageRenderer() {
    glGenTextures(1, &currentTexture);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
}

void LibretroDroid::ImageRenderer::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    // TODO We need to handle different texture types.
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB565, pitch / 2, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
}

uintptr_t LibretroDroid::ImageRenderer::getTexture() {
    return currentTexture;
}

uintptr_t LibretroDroid::ImageRenderer::getFramebuffer() {
    return 0; // ImageRender does not really expose a framebuffer.
}

LibretroDroid::FramebufferRenderer::FramebufferRenderer(unsigned width, unsigned height, bool depth, bool stencil) {
    glGenFramebuffers(1, &currentFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);

    glGenTextures(1, &currentTexture);
    glBindTexture(GL_TEXTURE_2D, currentTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, currentTexture, 0);

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

void LibretroDroid::FramebufferRenderer::onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) {
    // TODO Here something might be needed to handle frame duping.
}

uintptr_t LibretroDroid::FramebufferRenderer::getTexture() {
    return currentTexture;
}

uintptr_t LibretroDroid::FramebufferRenderer::getFramebuffer() {
    return currentFramebuffer;
}
