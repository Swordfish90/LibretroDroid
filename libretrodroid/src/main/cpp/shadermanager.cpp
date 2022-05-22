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

const std::string ShaderManager::upscaleVertex =
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

const std::string ShaderManager::upscaleFragment =
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
    "  return v.g;\n"
    "}\n"
    "lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {\n"
    "  return clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0);\n"
    "}\n"
    "lowp float sharpSmooth(lowp float t, lowp float sharpness) {\n"
    "  return linearStep(sharpness, 1.0 - sharpness, t);\n"
    "}\n"
    "lowp vec3 quadBilinear(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, lowp vec2 p, lowp float sharpness) {\n"
    "  lowp float x = sharpSmooth(p.x, sharpness);\n"
    "  lowp float y = sharpSmooth(p.y, sharpness);\n"
    "  return mix(mix(a, b, x), mix(c, d, x), y);\n"
    "}\n"
    "// Fast computation of barycentric coordinates only in the sub-triangle 1 2 4\n"
    "lowp vec3 fastBarycentric(lowp vec2 p, lowp float sharpness) {\n"
    "  lowp float l0 = sharpSmooth(1.0 - p.x - p.y, sharpness);\n"
    "  lowp float l1 = sharpSmooth(p.x, sharpness);\n"
    "  return vec3(l0, l1, 1.0 - l0 - l1);\n"
    "}\n"
    "\n"
    "lowp vec3 triangleInterpolate(lowp vec3 t1, lowp vec3 t2, lowp vec3 t3, lowp vec3 t4, lowp vec2 c, lowp float sharpness) {\n"
    "  // Alter colors and coordinates to compute the other triangle.\n"
    "  bool altTriangle = 1.0 - c.x < c.y;\n"
    "  lowp vec3 cornerColor = altTriangle ? t3 : t1;\n"
    "  lowp vec2 triangleCoords = altTriangle ? vec2(1.0 - c.y, 1.0 - c.x) : c;\n"
    "  lowp vec3 weights = fastBarycentric(triangleCoords, sharpness);\n"
    "  return weights.x * cornerColor + weights.y * t2 + weights.z * t4;\n"
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
    "  lowp float l1 = luma(t1);\n"
    "  lowp float l2 = luma(t2);\n"
    "  lowp float l3 = luma(t3);\n"
    "  lowp float l4 = luma(t4);\n"
    "\n"
    "#if USE_DYNAMIC_SHARPNESS\n"
    "  lowp float lmax = max(max(l1, l2), max(l3, l4));\n"
    "  lowp float lmin = min(min(l1, l2), min(l3, l4));\n"
    "  lowp float contrast = (lmax - lmin) / (lmax + lmin + 0.05);\n"
    "#if USE_SHARPENING_BIAS\n"
    "  contrast = sqrt(contrast);\n"
    "#endif\n"
    "  lowp float sharpness = mix(DYNAMIC_SHARPNESS_MIN, DYNAMIC_SHARPNESS_MAX, contrast);\n"
    "#else\n"
    "  const lowp float sharpness = STATIC_SHARPNESS;\n"
    "#endif\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  lowp float diagonal1Strength = abs(l1 - l3);\n"
    "  lowp float diagonal2Strength = abs(l2 - l4);\n"
    "\n"
    "  // Alter colors and coordinates to compute the other triangulation.\n"
    "  bool altTriangulation = diagonal1Strength < diagonal2Strength;\n"
    "\n"
    "  lowp vec3 cd = triangleInterpolate(\n"
    "    altTriangulation ? t2 : t1,\n"
    "    altTriangulation ? t3 : t2,\n"
    "    altTriangulation ? t4 : t3,\n"
    "    altTriangulation ? t1 : t4,\n"
    "    altTriangulation ? vec2(pxCoords.y, 1.0 - pxCoords.x) : pxCoords,\n"
    "    sharpness\n"
    "  );\n"
    "\n"
    "  lowp float minDiagonal = min(diagonal1Strength, diagonal2Strength);\n"
    "  lowp float maxDiagonal = max(diagonal1Strength, diagonal2Strength);\n"
    "  bool diagonal = minDiagonal * 2.0 + 0.05 < maxDiagonal;\n"
    "\n"
    "  lowp vec3 final = diagonal ? cd : quadBilinear(t1, t2, t4, t3, pxCoords, sharpness);\n"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";

ShaderManager::Data ShaderManager::getShader(Type type) {
    switch (type) {
    case Type::SHADER_DEFAULT:
        return { defaultShaderVertex, defaultShaderFragment, true };

    case Type::SHADER_CRT:
        return { defaultShaderVertex, crtShaderFragment, true };

    case Type::SHADER_LCD:
        return { defaultShaderVertex, lcdShaderFragment, true };

    case Type::SHADER_SHARP:
        return { defaultShaderVertex, defaultSharpFragment, true };

    case Type::SHADER_UPSCALE_SHARP:
        return {
            upscaleVertex,
            "#define USE_DYNAMIC_SHARPNESS 1\n"
            "#define USE_SHARPENING_BIAS 1\n"
            "#define DYNAMIC_SHARPNESS_MIN 0.10\n"
            "#define DYNAMIC_SHARPNESS_MAX 0.30\n"
            "#define STATIC_SHARPNESS 0.2\n"
            + upscaleFragment,
            false
        };

    case Type::SHADER_UPSCALE_SMOOTH:
        return {
            upscaleVertex,
            "#define USE_DYNAMIC_SHARPNESS 1\n"
            "#define USE_SHARPENING_BIAS 1\n"
            "#define DYNAMIC_SHARPNESS_MIN 0.00\n"
            "#define DYNAMIC_SHARPNESS_MAX 0.25\n"
            "#define STATIC_SHARPNESS 0.2\n"
            + upscaleFragment,
            false
        };
    }
}

} //namespace libretrodroid
