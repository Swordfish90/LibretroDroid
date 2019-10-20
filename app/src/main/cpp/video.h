//
// Created by swordfish on 18/10/19.
//

#ifndef HELLO_GL2_VIDEO_H
#define HELLO_GL2_VIDEO_H

#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>

#include "renderer.h"

namespace LibretroDroid {

class Video {
public:
    void initializeGraphics(Renderer* renderer, int screenWidth, int screenHeight, bool bottomLeftOrigin, float aspectRatio);

    void renderFrame();

    void onNewFrame(const void *data, unsigned width, unsigned height, size_t pitch);

    uintptr_t getCurrentFramebuffer() {
        return renderer->getFramebuffer();
    };

private:
    void updateCoords(unsigned width, unsigned height, size_t pitch);

    void updateVertices();

private:
    GLfloat gTriangleVertices[12];
    GLfloat gTriangleCoords[12];

    bool bottomLeftOrigin = false;
    int screenWidth = 0;
    int screenHeight = 0;
    float aspectRatio = 1.0F;

    GLuint gProgram = 0;
    GLint gvPositionHandle = 0;
    GLint gvCoordinateHandle = 0;
    GLint textureHandle = 0;

    Renderer* renderer;
};

}

#endif //HELLO_GL2_VIDEO_H
