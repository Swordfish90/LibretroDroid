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

const std::string ShaderManager::cutUpscaleVertex =
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
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 c1;\n"
    "varying HIGHP vec2 c2;\n"
    "varying HIGHP vec2 c3;\n"
    "varying HIGHP vec2 c4;\n"
    "\n"
    "void main() {\n"
    "  coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
//    "  coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001 * 0.2 + vec2(0.8, 0.0);\n"
    "  screenCoords = coords * textureSize;\n"
    "  c1 = (screenCoords) / textureSize;\n"
    "  c2 = (screenCoords + vec2(1.0, 0.0)) / textureSize;\n"
    "  c3 = (screenCoords + vec2(1.0, 1.0)) / textureSize;\n"
    "  c4 = (screenCoords + vec2(0.0, 1.0)) / textureSize;\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::string ShaderManager::cutUpscaleFragment =
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
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 c1;\n"
    "varying HIGHP vec2 c2;\n"
    "varying HIGHP vec2 c3;\n"
    "varying HIGHP vec2 c4;\n"
    "\n"
    "#if USE_FAST_LUMA\n"
    "lowp float luma(lowp vec3 v) {\n"
    "  return v.g;\n"
    "}\n"
    "#else\n"
    "lowp float luma(lowp vec3 v) {\n"
    "  return dot(v, vec3(0.299, 0.587, 0.114));\n"
    "}\n"
    "#endif\n"
    "\n"
    "lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {\n"
    "  return clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0);\n"
    "}\n"
    "lowp float sharpSmooth(lowp float edge0, lowp float edge1, lowp float t, lowp float sharpness) {\n"
    "  lowp float dist = edge1 - edge0;\n"
    "  lowp float dx = sharpness * dist;\n"
    "  return mix(edge0, edge1, linearStep(edge0 + dx, edge1 - dx, t));\n"
    "}\n"
    "lowp vec3 interpolate(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, lowp float tana, lowp vec2 p, lowp float sharpness) {\n"
    "  lowp vec4 w = vec4(0.0);\n"
    "  if (p.x < tana * (1.0 - p.y)) {\n"
    "    lowp float lambda1 = sharpSmooth(0.0, 1.0, 1.0 - p.x / tana - p.y, sharpness);\n"
    "    lowp float lambda2 = p.x / tana;\n"
    "    lowp float lambda3 = 1.0 - lambda1 - lambda2;\n"
    "    w = vec4(lambda1 + lambda2 * (1.0 - tana), lambda2 * tana, lambda3, 0.0);\n"
    "  } else if (p.x > 1.0 - tana * p.y) {\n"
    "    lowp float lambda1 = 1.0 - p.y;\n"
    "    lowp float lambda2 = sharpSmooth(0.0, 1.0, (p.x - 1.0 + p.y * tana) / tana, sharpness);\n"
    "    lowp float lambda3 = 1.0 - lambda1 - lambda2;\n"
    "    w = vec4(0.0, lambda1, lambda3 * tana, lambda2 + lambda3 * (1.0 - tana));\n"
    "  } else {\n"
    "    lowp float wu = sharpSmooth(tana, 1.0, p.x + tana * p.y, sharpness);\n"
    "    lowp float wd = sharpSmooth(0.0, 1.0 - tana, p.x - tana * (1.0 - p.y), sharpness);\n"
    "    lowp float wy = p.y;\n"
    "    w = vec4((1.0 - wu) * (1.0 - wy), wu * (1.0 - wy), (1.0 - wd) * wy, wd * wy);\n"
    "  }\n"
    "  return w.x * a + w.y * b + w.z * c + w.w * d;"
    "}\n"
    "\n"
    "lowp float round(lowp float value, lowp float levels) {\n"
    "  return floor(abs(value) * levels + 0.5) / levels;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 t1 = texture2D(texture, c1).rgb;\n"
    "  lowp vec3 t2 = texture2D(texture, c2).rgb;\n"
    "  lowp vec3 t3 = texture2D(texture, c3).rgb;\n"
    "  lowp vec3 t4 = texture2D(texture, c4).rgb;\n"

//    "  lowp vec3 t1 = vec3(0.5);\n"
//    "  lowp vec3 t2 = vec3(0.0);\n"
//    "  lowp vec3 t3 = vec3(1.0);\n"
//    "  lowp vec3 t4 = vec3(0.5);\n"

//    "  lowp vec3 t1 = vec3(1.0);\n"
//    "  lowp vec3 t3 = vec3(0.0);\n"
//    "  lowp vec3 t2 = vec3(0.0);\n"
//    "  lowp vec3 t4 = vec3(0.0);\n"

//    "  lowp vec3 t1 = vec3(0.9);\n"
//    "  lowp vec3 t2 = vec3(0.0);\n"
//    "  lowp vec3 t3 = vec3(0.1);\n"
//    "  lowp vec3 t4 = vec3(0.2);\n"

    "\n"
    "  lowp float l1 = luma(t1);\n"
    "  lowp float l2 = luma(t2);\n"
    "  lowp float l3 = luma(t3);\n"
    "  lowp float l4 = luma(t4);\n"
    "\n"
    "  lowp vec2 edgeDirection = vec2(l1 - l3, l2 - l4);\n"
    "  lowp vec2 tanDirection = vec2(edgeDirection.y - edgeDirection.x, edgeDirection.y + edgeDirection.x);\n"
    "  lowp vec2 absTanDirection = abs(tanDirection);\n"
    "  lowp float edgeStrength = dot(abs(edgeDirection), vec2(0.5));\n"
    "#if USE_DYNAMIC_SHARPNESS\n"
//    "  lowp float lmax = max(max(l1, l2), max(l3, l4));\n"
//    "  lowp float lmin = min(min(l1, l2), min(l3, l4));\n"
//    "  lowp float contrast = (lmax - lmin) / (lmax + lmin + 0.05);\n"
    "  lowp float contrast = edgeStrength;\n"
    "#if USE_SHARPENING_BIAS\n"
    "  contrast = sqrt(contrast);\n"
    "#endif\n"
    "  lowp float sharpness = mix(DYNAMIC_SHARPNESS_MIN, DYNAMIC_SHARPNESS_MAX, contrast);\n"
    "#else\n"
    "  const lowp float sharpness = STATIC_SHARPNESS;\n"
    "#endif\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
//    "  lowp vec2 pxCoords = fract(coords);\n"
    "\n"
    "  bool hasAngle = edgeStrength > 0.05;\n"
    "  bool invertedAngle = hasAngle && absTanDirection.y > absTanDirection.x;\n"
    "  bool negativeAngle = hasAngle && (tanDirection.x * tanDirection.y) >= 0.0;\n"
    "\n"
    "  lowp float tannum = min(absTanDirection.x, absTanDirection.y);\n"
    "  lowp float tanden = max(absTanDirection.x, absTanDirection.y);\n"
    "  lowp float atana = hasAngle ? tannum / (tanden + 0.01) : 0.0;\n"
    "\n"
//    "#ifdef MAX_LEVELS\n"
//    "  atana = round(atana, float(MAX_LEVELS));\n"
//    "#endif"
    "\n"
    "  lowp vec3 a = t1;\n"
    "  lowp vec3 b = invertedAngle ? t4 : t2;\n"
    "  lowp vec3 c = invertedAngle ? t2 : t4;\n"
    "  lowp vec3 d = t3;\n"
    "  pxCoords = invertedAngle ? pxCoords.yx : pxCoords;\n"
    "\n"
    "  lowp vec3 final = interpolate(\n"
    "    negativeAngle ? b : a,\n"
    "    negativeAngle ? a : b,\n"
    "    negativeAngle ? d : c,\n"
    "    negativeAngle ? c : d,\n"
    "    atana,\n"
    "    negativeAngle ? vec2(1.0 - pxCoords.x, pxCoords.y) : pxCoords,\n"
    "    sharpness\n"
    "  );\n"
    "\n"
//    "  lowp vec2 cs = fract(screenCoords);\n"
//    "  lowp float mask = (step(0.0, cs.x) - step(0.95, cs.x)) * (step(0.0, cs.y) - step(0.95, cs.y));\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
//    "  gl_FragColor = vec4(final * mask, 1.0);\n"
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

    case Type::SHADER_UPSCALE_CUT_SHARP:
        return {
            cutUpscaleVertex,
            "#define USE_DYNAMIC_SHARPNESS 1\n"
            "#define USE_SHARPENING_BIAS 1\n"
            "#define DYNAMIC_SHARPNESS_MIN 0.10\n"
            "#define DYNAMIC_SHARPNESS_MAX 0.30\n"
            "#define STATIC_SHARPNESS 0.5\n"
//            "#define MAX_LEVELS 2.0\n"
            "#define USE_FAST_LUMA 0\n"
            + cutUpscaleFragment,
            false
        };

    case Type::SHADER_UPSCALE_CUT_SMOOTH:
        return {
            cutUpscaleVertex,
            "#define USE_DYNAMIC_SHARPNESS 1\n"
            "#define USE_SHARPENING_BIAS 1\n"
            "#define DYNAMIC_SHARPNESS_MIN 0.00\n"
            "#define DYNAMIC_SHARPNESS_MAX 0.25\n"
            "#define STATIC_SHARPNESS 0.20\n"
            "#define USE_FAST_LUMA 0\n"
            + cutUpscaleFragment,
            false
        };
    }
}

} //namespace libretrodroid
