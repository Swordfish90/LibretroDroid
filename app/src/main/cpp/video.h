//
// Created by swordfish on 18/10/19.
//

#ifndef HELLO_GL2_VIDEO_H
#define HELLO_GL2_VIDEO_H

namespace LibretroDroid {

class Video {
public:
    void initializeGraphics(int screenWidth, int screenHeight);
    void renderFrame();
    void hw_initialize_framebuffer(int width, int height, bool depth, bool stencil);
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

    const GLfloat gTriangleCoords[12] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f
    };

    int screenWidth = 0;
    int screenHeight = 0;

    // OpenGL parameters
    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint textureHandle = 0;

    // HW Accelerated parameters
    unsigned current_framebuffer = 0;
    unsigned current_framebuffer_texture = 0;
};

}

#endif //HELLO_GL2_VIDEO_H
