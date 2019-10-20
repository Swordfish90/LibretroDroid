//
// Created by swordfish on 18/10/19.
//

#ifndef HELLO_GL2_VIDEO_H
#define HELLO_GL2_VIDEO_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

namespace LibretroDroid {

class Video {
public:
    void initializeGraphics(int screenWidth, int screenHeight, bool bottomLeftOrigin);
    void renderFrame();
    void initialize2DRendering(int width, int height, float aspectRatio);
    void initialize3DRendering(int width, int height, float aspectRatio, bool depth, bool stencil);

    void onNew2DFrame(const void *data, unsigned width, unsigned height, size_t pitch);
    void onNew3DFrame(const void *data, unsigned width, unsigned height, size_t pitch);

    uintptr_t getCurrentFramebuffer() { return current_framebuffer; };

private:
    GLfloat gTriangleVertices[12] = {
            -1.0f, -1.0f,
            -1.0f,1.0f,
            1.0f, -1.0f,
            1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, 1.0f
    };

    GLfloat gTriangleCoords[12] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
    };

    void updateCoords(unsigned width, unsigned height, size_t pitch);
    void updateVertices();

    bool bottomLeftOrigin = false;
    int screenWidth = 0;
    int screenHeight = 0;
    float aspectRatio = 1.0F;

    // OpenGL parameters
    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint textureHandle = 0;

    // HW Accelerated parameters
    unsigned current_framebuffer = 0;
    unsigned current_framebuffer_texture = 0;

    // Non HW accelerated parameters
    unsigned current_texture = 0;

    bool use3DRendering = false;
};

}

#endif //HELLO_GL2_VIDEO_H
