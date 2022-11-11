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

const std::string ShaderManager::cut2UpscaleVertex =
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
    "varying HIGHP vec2 c1;\n"
    "varying HIGHP vec2 c2;\n"
    "varying HIGHP vec2 c3;\n"
    "varying HIGHP vec2 c4;\n"
    "varying lowp float displaySharpness;\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  c1 = (screenCoords) / textureSize;\n"
    "  c2 = (screenCoords + vec2(1.0, 0.0)) / textureSize;\n"
    "  c3 = (screenCoords + vec2(1.0, 1.0)) / textureSize;\n"
    "  c4 = (screenCoords + vec2(0.0, 1.0)) / textureSize;\n"
    "  displaySharpness = SHARPNESS_MAX * (0.5 - 0.5 / max(screenDensity, 1.0));\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::unordered_map<std::string, std::string> ShaderManager::cut2UpscaleParams = {
    { "SHARPNESS_BIAS", "1.0" },
    { "SHARPNESS_MAX", "1.0" },
    { "USE_FAST_LUMA", "1" }
};

const std::string ShaderManager::cut2UpscaleFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "#define EPSILON 0.01\n"
    "\n"
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float screenDensity;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying HIGHP vec2 c1;\n"
    "varying HIGHP vec2 c2;\n"
    "varying HIGHP vec2 c3;\n"
    "varying HIGHP vec2 c4;\n"
    "varying lowp float displaySharpness;\n"
    "\n"
    "lowp float fastLuma(lowp vec3 v) {\n"
    "  return v.g;\n"
    "}\n"
    "#if USE_FAST_LUMA\n"
    "lowp float luma(lowp vec3 v) {\n"
    "  return fastLuma(v);\n"
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
    "lowp float sharpSmooth(lowp float t, lowp float sharpness) {\n"
    "  return linearStep(sharpness, 1.0 - sharpness, t);\n"
    "}\n"
    "lowp vec3 blend(lowp vec3 a, lowp vec3 b, lowp float t) {\n"
    "  lowp float lumaDiff = abs(fastLuma(a) - fastLuma(b));\n"
    "  lowp float sharpness = displaySharpness * min(lumaDiff * SHARPNESS_BIAS, 1.0);\n"
    "  return mix(a, b, sharpSmooth(t, sharpness));\n"
    "}\n"
    "lowp vec3 interpolate(\n"
    "  lowp vec3 a,\n"
    "  lowp vec3 b,\n"
    "  lowp vec3 c,\n"
    "  lowp vec3 d,\n"
    "  lowp float tana,\n"
    "  lowp vec2 p\n"
    ") {\n"
    "  bool leftTriangle = p.x < tana * (1.0 - p.y);\n"
    "  bool rightTriangle = p.x > 1.0 - tana * p.y;\n"
    "  lowp vec3 p0 = rightTriangle ? b : a;\n"
    "  lowp vec3 p1 = rightTriangle ? d : b;\n"
    "  lowp vec3 p2 = leftTriangle ? a : c;\n"
    "  lowp vec3 p3 = leftTriangle ? c : d;\n"
    "  lowp float x0 = rightTriangle ? p.y - (1.0 - p.x) / tana : p.x + p.y * tana;\n"
    "  lowp float x1 = leftTriangle ? p.y + p.x / tana : p.x - (1.0 - p.y) * tana;\n"
    "  lowp float y = \n"
    "    leftTriangle ? p.y / x1 :\n"
    "    rightTriangle ? (p.y - x0) / (1.0 - x0) :\n"
    "    p.y;\n"
    "  return blend(blend(p0, p1, x0), blend(p2, p3, x1), y);\n"
    "}\n"
    "\n"
    "void main() {\n"
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
    "  lowp vec2 gradient = vec2(l1 - l2 - l3 + l4, l1 + l2 - l3 - l4);\n"
    "  lowp vec2 absGradient = abs(gradient);\n"
    "\n"
    "  bool invertedAngle = absGradient.y > absGradient.x + EPSILON;\n"
    "  bool negativeAngle = (gradient.x * gradient.y) < -EPSILON * EPSILON;\n"
    "\n"
    "  lowp float minGradient = min(absGradient.x, absGradient.y);\n"
    "  lowp float maxGradient = max(absGradient.x, absGradient.y);\n"
    "  lowp float atana = step(maxGradient + EPSILON, minGradient * 2.0);\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
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
    "    negativeAngle ? vec2(1.0 - pxCoords.x, pxCoords.y) : pxCoords\n"
    "  );\n"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";

const std::unordered_map<std::string, std::string> ShaderManager::cutUpscaleParams = {
    { "USE_DYNAMIC_SHARPNESS", "1" },
    { "USE_SHARPENING_BIAS", "1" },
    { "DYNAMIC_SHARPNESS_MIN", "0.10" },
    { "DYNAMIC_SHARPNESS_MAX", "0.30" },
    { "STATIC_SHARPNESS", "0.5" },
    { "USE_FAST_LUMA", "1" },
    { "TRIANGULATION_THRESHOLD", "4.0" },
};

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
    "varying HIGHP vec2 c1;\n"
    "varying HIGHP vec2 c2;\n"
    "varying HIGHP vec2 c3;\n"
    "varying HIGHP vec2 c4;\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
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
    "  bool diagonal = minDiagonal * TRIANGULATION_THRESHOLD + 0.05 < maxDiagonal;\n"
    "\n"
    "  lowp vec3 final = diagonal ? cd : quadBilinear(t1, t2, t4, t3, pxCoords, sharpness);\n"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";


ShaderManager::Data ShaderManager::getShader(const ShaderManager::Config& config) {
    switch (config.type) {
    case Type::SHADER_DEFAULT: {
        return {defaultShaderVertex, defaultShaderFragment, true};
    }

    case Type::SHADER_CRT: {
        return {defaultShaderVertex, crtShaderFragment, true};
    }

    case Type::SHADER_LCD: {
        return {defaultShaderVertex, lcdShaderFragment, true};
    }

    case Type::SHADER_SHARP: {
        return {defaultShaderVertex, defaultSharpFragment, true};
    }

    case Type::SHADER_UPSCALE_CUT: {
        std::string defines = buildDefines(cutUpscaleParams, config.params);
        return {
            defines + cutUpscaleVertex,
            defines + cutUpscaleFragment,
            false
        };
    }

    case Type::SHADER_UPSCALE_CUT2: {
        std::string defines = buildDefines(cut2UpscaleParams, config.params);
        return {
            defines + cut2UpscaleVertex,
            defines + cut2UpscaleFragment,
            false
        };
    }
}
}

std::string ShaderManager::buildDefines(
    std::unordered_map<std::string, std::string> baseParams,
    std::unordered_map<std::string, std::string> customParams
) {
    customParams.insert(baseParams.begin(), baseParams.end());

    std::string result;
    std::for_each(customParams.begin(), customParams.end(), [&result] (auto param) {
        result += "#define " + param.first + " " + param.second + "\n";
    });

    return result + "\n";
}

} //namespace libretrodroid
