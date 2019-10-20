//
// Created by swordfish on 20/10/19.
//

#ifndef HELLO_GL2_RENDERER_H
#define HELLO_GL2_RENDERER_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <cstdint>

namespace LibretroDroid {

class Renderer {
public:
    virtual uintptr_t getFramebuffer() = 0;
    virtual uintptr_t getTexture() = 0;
    virtual void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch) = 0;
};

class ImageRenderer: public Renderer {
public:
    ImageRenderer();
    uintptr_t getTexture();
    uintptr_t getFramebuffer();
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

private:
    GLuint currentTexture = 0;
};

class FramebufferRenderer: public Renderer {
public:
    FramebufferRenderer(unsigned width, unsigned height, bool depth, bool stencil);
    uintptr_t getTexture();
    uintptr_t getFramebuffer();
    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

private:
    GLuint currentFramebuffer = 0;
    GLuint currentTexture = 0;
};

}


#endif //HELLO_GL2_RENDERER_H
