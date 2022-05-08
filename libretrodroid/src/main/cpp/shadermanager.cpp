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

#include "shadermanager.h"

namespace libretrodroid {

const std::string ShaderManager::defaultShaderVertex =
    "attribute vec4 vPosition;\n"
    "attribute vec2 vCoordinate;\n"
    "uniform mediump float vFlipY;\n"
    "uniform mediump float screenDensity;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mat4 vViewModel;\n"
    "uniform vec2 textureSize;\n"
    "\n"
    "varying mediump float screenMaskStrength;\n"
    "varying vec2 coords;\n"
    "varying vec2 screenCoords;\n"
    "void main() {\n"
    "  coords.x = vCoordinate.x;\n"
    "  coords.y = mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY);\n"
    "  screenCoords = coords * textureSize;\n"
    "  screenMaskStrength = smoothstep(2.0, 6.0, screenDensity);\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::string ShaderManager::defaultShaderFragment =
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "varying vec2 coords;\n"
    "void main() {\n"
    "  vec4 tex = texture2D(texture, coords);"
    "  gl_FragColor = vec4(tex.rgb, 1.0);\n"
    "}\n";

const std::string ShaderManager::crtShaderFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform HIGHP vec2 textureSize;\n"
    "\n"
    "uniform lowp sampler2D texture;\n"
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying mediump float screenMaskStrength;\n"
    "\n"
    "#define INTENSITY 0.30\n"
    "#define BRIGHTBOOST 0.30\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 texel = texture2D(texture, coords).rgb;\n"
    "  lowp vec3 pixelHigh = ((1.0 + BRIGHTBOOST) - (0.2 * texel)) * texel;\n"
    "  lowp vec3 pixelLow  = ((1.0 - INTENSITY) + (0.1 * texel)) * texel;\n"
    "\n"
    "  HIGHP vec2 coords = fract(screenCoords) * 2.0 - vec2(1.0);\n"
    "\n"
    "  lowp float mask = 1.0 - abs(coords.y);\n"
    "\n"
    "  gl_FragColor = vec4(mix(texel, mix(pixelLow, pixelHigh, mask), screenMaskStrength), 1.0);\n"
    "}\n";

const std::string ShaderManager::lcdShaderFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mediump float screenDensity;\n"
    "\n"
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying mediump float screenMaskStrength;\n"
    "\n"
    "#define INTENSITY 0.25\n"
    "#define BRIGHTBOOST 0.25\n"
    "\n"
    "void main() {\n"
    "  mediump vec2 threshold = vec2(1.0 / screenDensity);\n"
    "  mediump vec2 x = fract(screenCoords);\n"
    "  x = 0.5 * (smoothstep(vec2(0.0), threshold, x) + smoothstep(vec2(1.0) - threshold, vec2(1.0), x));\n"
    "  mediump vec2 sharpCoords = (floor(screenCoords) + x) / textureSize;\n"
    "\n"
    "  lowp vec3 texel = texture2D(texture, sharpCoords).rgb;\n"
    "  lowp vec3 pixelHigh = ((1.0 + BRIGHTBOOST) - (0.2 * texel)) * texel;\n"
    "  lowp vec3 pixelLow  = ((1.0 - INTENSITY) + (0.1 * texel)) * texel;\n"
    "\n"
    "  HIGHP vec2 coords = fract(screenCoords) * 2.0 - vec2(1.0);\n"
    "  coords = coords * coords;\n"
    "\n"
    "  lowp float mask = 1.0 - coords.x - coords.y;\n"
    "\n"
    "  gl_FragColor = vec4(mix(texel, mix(pixelLow, pixelHigh, mask), screenMaskStrength), 1.0);\n"
    "}\n";

const std::string ShaderManager::defaultSharpFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float screenDensity;\n"
    "\n"
    "varying vec2 coords;\n"
    "varying vec2 screenCoords;\n"
    "\n"
    "void main() {\n"
    "  mediump vec2 threshold = vec2(1.0 / screenDensity);\n"
    "  mediump vec2 x = fract(screenCoords);\n"
    "  x = 0.5 * (smoothstep(vec2(0.0), threshold, x) + smoothstep(vec2(1.0) - threshold, vec2(1.0), x));\n"
    "  mediump vec2 sharpCoords = (floor(screenCoords) + x) / textureSize;\n"
    "\n"
    "  vec4 tex = texture2D(texture, sharpCoords);\n"
    "  gl_FragColor = vec4(tex.rgb, 1.0);\n"
    "}\n";

const std::string ShaderManager::triangleUpscaleVertex =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "attribute vec4 vPosition;\n"
    "attribute vec2 vCoordinate;\n"
    "uniform mediump float vFlipY;\n"
    "uniform mediump float screenDensity;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mat4 vViewModel;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "void main() {\n"
    "  mediump vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.000001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::string ShaderManager::triangleUpscaleFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "#define SAME_COLOR 0.1\n"
    "\n"
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float screenDensity;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "\n"
    "lowp float dist(lowp vec3 v1, lowp vec3 v2) {\n"
    "  return length(v1 - v2);\n"
    "}\n"
    "\n"
    "lowp vec3 nearest(lowp vec3 p1, lowp vec3 p2, lowp vec3 p3, lowp vec3 p4, mediump vec2 c) {\n"
    "  lowp vec3 v1 = mix(p1, p2, step(0.5, c.x));\n"
    "  lowp vec3 v2 = mix(p4, p3, step(0.5, c.x));\n"
    "  return mix(v1, v2, step(0.5, c.y));\n"
    "}\n"
    "lowp float diamond(mediump vec2 c) {\n"
    "  return step(c.x, c.y + 0.5) * \n"
    "    step(c.x, 1.0 - c.y + 0.5) * \n"
    "    step(1.0 - c.x, c.y + 0.5) * \n"
    "    step(c.y, c.x + 0.5);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 relativeCoords = floor(screenCoords);\n"
    "  mediump vec2 c1 = ((relativeCoords + vec2(0.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c2 = ((relativeCoords + vec2(1.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c3 = ((relativeCoords + vec2(1.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c4 = ((relativeCoords + vec2(0.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "\n"
    "  lowp vec3 t1 = texture2D(texture, c1).rgb;\n"
    "  lowp vec3 t2 = texture2D(texture, c2).rgb;\n"
    "  lowp vec3 t3 = texture2D(texture, c3).rgb;\n"
    "  lowp vec3 t4 = texture2D(texture, c4).rgb;\n"
    "\n"
    "  mediump vec2 pxCoords = fract(screenCoords);\n"
    "  lowp vec3 final = nearest(t1, t2, t3, t4, pxCoords);\n"
    "\n"
    "  lowp float d1 = 1.0 - step(SAME_COLOR, dist(t1, t3));\n"
    "  lowp float d2 = 1.0 - step(SAME_COLOR, dist(t2, t4));\n"
    "\n"
    "  lowp vec3 p1 = mix(t1, t3, 0.5);\n"
    "  lowp vec3 p2 = mix(t2, t4, 0.5);\n"
    "\n"
    "  lowp float mask = diamond(pxCoords);\n"
    "\n"
    "  final = mix(final, p1, d1 * (1.0 - d2) * mask);"
    "  final = mix(final, p2, d2 * (1.0 - d1) * mask);"
    "  final = mix(final, mix(p1, p2, 0.5), step(1.5, d1 + d2) * mask);"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";

const std::string ShaderManager::triangleUpscaleSmoothFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float screenDensity;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "\n"
    "lowp float rgb2grey(lowp vec3 v) {\n"
    "  return dot(v, vec3(0.21, 0.72, 0.04));\n"
    "}\n"
    "lowp vec3 barycentric(mediump vec2 a, mediump vec2 b, mediump vec2 c, mediump vec2 p) {\n"
    "  lowp float l0 = ((b.y-c.y)*(p.x-c.x) + (c.x-b.x)*(p.y-c.y))\n"
    "    / ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));\n"
    "  lowp float l1 = ((c.y-a.y)*(p.x-c.x)+(a.x-c.x)*(p.y-c.y))\n"
    "    / ((b.y-c.y)*(a.x-c.x)+(c.x-b.x)*(a.y-c.y));\n"
    "  return vec3(l0, l1, 1.0 - l0 - l1);\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 relativeCoords = floor(screenCoords);\n"
    "  mediump vec2 c1 = ((relativeCoords + vec2(0.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c2 = ((relativeCoords + vec2(1.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c3 = ((relativeCoords + vec2(1.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c4 = ((relativeCoords + vec2(0.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "\n"
    "  lowp vec3 t1 = texture2D(texture, c1).rgb;\n"
    "  lowp vec3 t2 = texture2D(texture, c2).rgb;\n"
    "  lowp vec3 t3 = texture2D(texture, c3).rgb;\n"
    "  lowp vec3 t4 = texture2D(texture, c4).rgb;\n"
    "\n"
    "  mediump vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  lowp vec3 wd11 = barycentric(vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), pxCoords);\n"
    "  lowp vec3 cd11 = (wd11.x * t1 + wd11.y * t4 + wd11.z * t3);\n"
    "  lowp vec3 wd12 = barycentric(vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), pxCoords);\n"
    "  lowp vec3 cd12 = (wd12.x * t1 + wd12.y * t2 + wd12.z * t3);\n"
    "  lowp vec3 cd1 = mix(cd12, cd11, step(pxCoords.x, pxCoords.y));\n"
    "\n"
    "  lowp vec3 wd21 = barycentric(vec2(0.0, 1.0), vec2(0.0, 0.0), vec2(1.0, 0.0), pxCoords);\n"
    "  lowp vec3 cd21 = (wd21.x * t4 + wd21.y * t1 + wd21.z * t2);\n"
    "  lowp vec3 wd22 = barycentric(vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0), pxCoords);\n"
    "  lowp vec3 cd22 = (wd22.x * t4 + wd22.y * t3 + wd22.z * t2);\n"
    "  lowp vec3 cd2 = mix(cd22, cd21, step(pxCoords.y, 1.0 - pxCoords.x));\n"
    "\n"
    "  lowp float d1 = abs(rgb2grey(t1) - rgb2grey(t3));\n"
    "  lowp float d2 = abs(rgb2grey(t2) - rgb2grey(t4));\n"
    "\n"
    "  lowp vec3 final = mix(cd1, cd2, step(d2, d1));\n"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";

std::tuple<std::string, std::string> ShaderManager::getShader(Type type) {
    switch (type) {
    case Type::SHADER_DEFAULT:
        return { defaultShaderVertex, defaultShaderFragment };

    case Type::SHADER_CRT:
        return { defaultShaderVertex, crtShaderFragment };

    case Type::SHADER_LCD:
        return { defaultShaderVertex, lcdShaderFragment };

    case Type::SHADER_SHARP:
        return { defaultShaderVertex, defaultSharpFragment };

    case Type::SHADER_TRIANGLE_UPSCALE:
        return { triangleUpscaleVertex, triangleUpscaleFragment };

    case Type::SHADER_TRIANGLE_UPSCALE_SMOOTH:
        return {triangleUpscaleVertex, triangleUpscaleSmoothFragment };
    }
}

} //namespace libretrodroid
