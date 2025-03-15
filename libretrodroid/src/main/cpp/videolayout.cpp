/*
 *     Copyright (C) 2025  Filippo Scognamiglio
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

#include "videolayout.h"
#include "log.h"

namespace libretrodroid {

VideoLayout::VideoLayout(float rotation, Rect viewportRect) :
    viewportRect(viewportRect),
    rotation(rotation)
{
    updateForegroundVertices();
}

std::array<float, 12>& VideoLayout::getForegroundVertices() {
    return videoVertices;
}

// TODO BLUR... We should probably rotate also the background...
void VideoLayout::updateForegroundVertices() {
    LOGD(
        "Computing foreground vertices from screen (%d x %d), aspect ratio (%f) with rotation (%f radians)",
        screenWidth, screenHeight, aspectRatio, rotation);

    float screenW = screenWidth * viewportRect.getWidth();
    float screenH = screenHeight * viewportRect.getHeight();
    float screenAspect = screenW / screenH;
    float contentAspect = aspectRatio;

    float scaleX = viewportRect.getWidth();
    float scaleY = viewportRect.getHeight();
    if (contentAspect > screenAspect) {
        scaleY *= screenAspect / contentAspect;
    } else {
        scaleX *= contentAspect / screenAspect;
    }

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

    videoVertices[0] = rotatedQuad[0][0];
    videoVertices[1] = rotatedQuad[0][1];

    videoVertices[2] = rotatedQuad[1][0];
    videoVertices[3] = rotatedQuad[1][1];

    videoVertices[4] = rotatedQuad[2][0];
    videoVertices[5] = rotatedQuad[2][1];

    videoVertices[6] = rotatedQuad[2][0];
    videoVertices[7] = rotatedQuad[2][1];

    videoVertices[8] = rotatedQuad[1][0];
    videoVertices[9] = rotatedQuad[1][1];

    videoVertices[10] = rotatedQuad[3][0];
    videoVertices[11] = rotatedQuad[3][1];
}

void libretrodroid::VideoLayout::updateAspectRatio(float aspectRatio) {
    LOGD("Updated aspect ratio to : %f", aspectRatio);

    this->aspectRatio = aspectRatio;
    updateForegroundVertices();
}

void libretrodroid::VideoLayout::updateScreenSize(unsigned int width,unsigned int height) {
    LOGD("Updating screen size: %d x %d", screenWidth, screenHeight);

    this->screenWidth = width;
    this->screenHeight = height;
    updateForegroundVertices();
}

void libretrodroid::VideoLayout::updateViewportSize(Rect viewport) {
    LOGD("Updating viewport size: (%f, %f, %f, %f)", viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());

    this->viewportRect = viewport;
    updateForegroundVertices();
}

void VideoLayout::updateRotation(float rotation) {
    LOGD("Updated rotation to : %f", rotation);

    this->rotation = rotation;
    updateForegroundVertices();
}

std::pair<float, float> VideoLayout::getRelativePosition(float touchX, float touchY) {
    float xMin = std::numeric_limits<float>::max();
    float xMax = std::numeric_limits<float>::lowest();
    float yMin = std::numeric_limits<float>::max();
    float yMax = std::numeric_limits<float>::lowest();

    for (size_t i = 0; i < videoVertices.size(); i += 2) {
        float x = videoVertices[i];
        float y = videoVertices[i + 1];
        xMin = std::min(xMin, x);
        xMax = std::max(xMax, x);
        yMin = std::min(yMin, y);
        yMax = std::max(yMax, y);
    }

    if (touchX < xMin || touchX > xMax || touchY < yMin || touchY > yMax) {
        return {-10.0f, -10.0f};
    }

    float relativeX = (touchX - xMin) / (xMax - xMin);
    float relativeY = (touchY - yMin) / (yMax - yMin);

    return {relativeX, relativeY};
}

}