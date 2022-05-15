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

const std::string ShaderManager::diamondUpscaleVertex =
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
    "\n"
    "void main() {\n"
    "  mediump vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.000001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::string ShaderManager::diamondUpscaleFragment =
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
    "lowp float luma(lowp vec3 v) {\n"
    "  return dot(v, vec3(0.21, 0.72, 0.04));\n"
    "}\n"
    "lowp float sharpSmooth(float t) {"
    "  return smoothstep(0.5 - SMOOTHNESS, 0.5 + SMOOTHNESS, t);"
    "}\n"
    "lowp vec3 quadBilinear(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, mediump vec2 p) {\n"
    "  return mix(mix(a, b, sharpSmooth(p.x)), mix(c, d, sharpSmooth(p.x)), sharpSmooth(p.y));\n"
    "}\n"
    "lowp vec3 barycentric(mediump vec2 a, mediump vec2 b, mediump vec2 c, mediump vec2 p) {\n"
    "  mediump vec2 dpc = p - c;\n"
    "  mediump vec2 dbc = b - c;\n"
    "  mediump vec2 dac = a - c;\n"
    "  mediump float denom = dbc.y * dac.x - dbc.x * dac.y;"
    "  lowp float l0 = sharpSmooth((dbc.y * dpc.x - dbc.x * dpc.y) / denom);\n"
    "  lowp float l1 = sharpSmooth((-dac.y * dpc.x + dac.x * dpc.y) / denom);\n"
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
    // Color when splitting along the diagonal 1 - 3
    "  lowp vec3 wd11 = barycentric(vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), pxCoords);\n"
    "  lowp vec3 cd11 = (wd11.x * t1 + wd11.y * t4 + wd11.z * t3);\n"
    "  lowp vec3 wd12 = barycentric(vec2(0.0, 0.0), vec2(1.0, 0.0), vec2(1.0, 1.0), pxCoords);\n"
    "  lowp vec3 cd12 = (wd12.x * t1 + wd12.y * t2 + wd12.z * t3);\n"
    "  lowp vec3 cd1 = mix(cd12, cd11, step(pxCoords.x, pxCoords.y));\n"
    "\n"
    // Color when splitting along the diagonal 2 - 4
    "  lowp vec3 wd21 = barycentric(vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 0.0), pxCoords);\n"
    "  lowp vec3 cd21 = (wd21.x * t1 + wd21.y * t4 + wd21.z * t2);\n"
    "  lowp vec3 wd22 = barycentric(vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, 0.0), pxCoords);\n"
    "  lowp vec3 cd22 = (wd22.x * t4 + wd22.y * t3 + wd22.z * t2);\n"
    "  lowp vec3 cd2 = mix(cd22, cd21, step(pxCoords.y, 1.0 - pxCoords.x));\n"
    "\n"
    "  lowp float diagonal1Strength = abs(luma(t1) - luma(t3));\n"
    "  lowp float diagonal2Strength = abs(luma(t2) - luma(t4));\n"
    "  lowp float d1 = step(diagonal2Strength * 3.0 + 0.02, diagonal1Strength);\n"
    "  lowp float d2 = step(diagonal1Strength * 3.0 + 0.02, diagonal2Strength);\n"
    "\n"
    "  lowp vec3 final = quadBilinear(t1, t2, t4, t3, pxCoords);\n"
    "  final = mix(final, cd2, d1 * (1.0 - d2))\n;"
    "  final = mix(final, cd1, d2 * (1.0 - d1))\n;"
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

    case Type::SHADER_DIAMOND_UPSCALE_SHARP:
        return { diamondUpscaleVertex, "#define SMOOTHNESS 0.2\n\n" + diamondUpscaleFragment };

    case Type::SHADER_DIAMOND_UPSCALE_SMOOTH:
        return { diamondUpscaleVertex, "#define SMOOTHNESS 0.4\n\n" + diamondUpscaleFragment };
    }
}

} //namespace libretrodroid
