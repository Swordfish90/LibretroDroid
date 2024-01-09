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
    "uniform vec2 textureSize;\n"
    "\n"
    "varying mediump float screenMaskStrength;\n"
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 screenCoords;\n"
    "void main() {\n"
    "  coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY));\n"
    "  screenCoords = coords * textureSize;\n"
    "  screenMaskStrength = smoothstep(2.0, 6.0, screenDensity);\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}\n";

const std::string ShaderManager::defaultShaderFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "uniform lowp sampler2D texture;\n"
    "varying HIGHP vec2 coords;\n"
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
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 screenCoords;\n"
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

const std::unordered_map<std::string, std::string> ShaderManager::cutUpscaleParams = {
    { "USE_DYNAMIC_BLEND", "1" },
    { "BLEND_MIN_CONTRAST_EDGE", "0.25" },
    { "BLEND_MAX_CONTRAST_EDGE", "0.75" },
    { "BLEND_MIN_SHARPNESS", "0.0" },
    { "BLEND_MAX_SHARPNESS", "1.0" },
    { "STATIC_BLEND_SHARPNESS", "1.0" },
    { "EDGE_USE_FAST_LUMA", "0" },
    { "EDGE_MIN_VALUE", "0.03" },
    { "EDGE_MIN_CONTRAST", "2.0" },
    { "LUMA_ADJUST_GAMMA", "0" },
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
    "uniform mediump float screenDensity;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mat4 vViewModel;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float vFlipY;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  c05 = (screenCoords + vec2(+0.0, +0.0)) / textureSize;\n"
    "  c06 = (screenCoords + vec2(+1.0, +0.0)) / textureSize;\n"
    "  c09 = (screenCoords + vec2(+0.0, +1.0)) / textureSize;\n"
    "  c10 = (screenCoords + vec2(+1.0, +1.0)) / textureSize;\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}";

const std::string ShaderManager::cutUpscaleFragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "#define EPSILON 0.02\n"
    "\n"
    "precision mediump float;\n"
    "uniform lowp sampler2D texture;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "\n"
    "lowp float luma(lowp vec3 v) {\n"
    "#if EDGE_USE_FAST_LUMA\n"
    "  lowp float result = v.g;\n"
    "#else\n"
    "  lowp float result = dot(v, vec3(0.299, 0.587, 0.114));\n"
    "#endif\n"
    "#if LUMA_ADJUST_GAMMA\n"
    "  result = sqrt(result);\n"
    "#endif\n"
    "  return result;\n"
    "}\n"
    "\n"
    "struct Pixels {\n"
    "  lowp vec3 p0;\n"
    "  lowp vec3 p1;\n"
    "  lowp vec3 p2;\n"
    "  lowp vec3 p3;\n"
    "};\n"
    "\n"
    "struct Pattern {\n"
    "  Pixels pixels;\n"
    "  bool triangle;\n"
    "  lowp vec2 coords;\n"
    "};\n"
    "\n"
    "lowp vec3 triangle(lowp vec2 pxCoords) {\n"
    "  lowp vec3 ws = vec3(0.0);\n"
    "  ws.x = pxCoords.y - pxCoords.x;\n"
    "  ws.y = 1.0 - ws.x;\n"
    "  ws.z = (pxCoords.y - ws.x) / (ws.y + EPSILON);\n"
    "  return ws;\n"
    "}\n"
    "\n"
    "lowp vec3 quad(lowp vec2 pxCoords) {\n"
    "  return vec3(pxCoords.x, pxCoords.x, pxCoords.y);\n"
    "}\n"
    "\n"
    "lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {\n"
    "  return clamp((t - edge0) / (edge1 - edge0 + EPSILON), 0.0, 1.0);\n"
    "}\n"
    "\n"
    "lowp float sharpSmooth(lowp float t, lowp float sharpness) {\n"
    "  return linearStep(sharpness, 1.0 - sharpness, t);\n"
    "}\n"
    "\n"
    "lowp float sharpness(lowp float l1, lowp float l2) {\n"
    "#if USE_DYNAMIC_BLEND\n"
    "  lowp float lumaDiff = abs(l1 - l2);\n"
    "  lowp float contrast = linearStep(BLEND_MIN_CONTRAST_EDGE, BLEND_MAX_CONTRAST_EDGE, lumaDiff);\n"
    "  lowp float result = mix(BLEND_MIN_SHARPNESS * 0.5, BLEND_MAX_SHARPNESS * 0.5, contrast);\n"
    "#else\n"
    "  lowp float result = STATIC_BLEND_SHARPNESS * 0.5;\n"
    "#endif\n"
    "  return result;\n"
    "}\n"
    "\n"
    "bool hasDiagonal(lowp float a, lowp float b, lowp float c, lowp float d) {\n"
    "  return max(distance(a, d) * EDGE_MIN_CONTRAST, EDGE_MIN_VALUE) < distance(b, c);\n"
    "}\n"
    "\n"
    "lowp vec3 blend(lowp vec3 a, lowp vec3 b, lowp float t) {\n"
    "  return mix(a, b, sharpSmooth(t, sharpness(luma(a), luma(b))));\n"
    "}\n"
    "\n"
    "Pattern pattern0(Pixels pixels, lowp vec2 pxCoords) {\n"
    "  return Pattern(pixels, false, pxCoords);\n"
    "}\n"
    "\n"
    "Pattern pattern1(Pixels pixels, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "  if (pxCoords.y > pxCoords.x) {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p2, pixels.p2, pixels.p3);\n"
    "    result.triangle = true;\n"
    "    result.coords = vec2(pxCoords.x, pxCoords.y);\n"
    "  } else {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p1, pixels.p1, pixels.p3);\n"
    "    result.triangle = true;\n"
    "    result.coords = vec2(pxCoords.y, pxCoords.x);\n"
    "  }\n"
    "  return result;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 t05 = texture2D(texture, c05).rgb;\n"
    "  lowp vec3 t06 = texture2D(texture, c06).rgb;\n"
    "  lowp vec3 t09 = texture2D(texture, c09).rgb;\n"
    "  lowp vec3 t10 = texture2D(texture, c10).rgb;\n"
    "\n"
    "  lowp float l05 = luma(t05);\n"
    "  lowp float l06 = luma(t06);\n"
    "  lowp float l09 = luma(t09);\n"
    "  lowp float l10 = luma(t10);\n"
    "\n"
    "  Pixels pixels = Pixels(t05, t06, t09, t10);\n"
    "\n"
    "  bool d05_10 = hasDiagonal(l05, l06, l09, l10);\n"
    "  bool d06_09 = hasDiagonal(l06, l05, l10, l09);\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  if (d06_09) {\n"
    "    pixels = Pixels(pixels.p1, pixels.p0, pixels.p3, pixels.p2);\n"
    "    pxCoords.x = 1.0 - pxCoords.x;\n"
    "  }\n"
    "\n"
    "  Pattern pattern;\n"
    "\n"
    "  if (d05_10 || d06_09) {\n"
    "    pattern = pattern1(pixels, pxCoords);\n"
    "  } else {\n"
    "    pattern = pattern0(pixels, pxCoords);\n"
    "  }\n"
    "\n"
    "  lowp vec3 weights = pattern.triangle ? triangle(pattern.coords) : quad(pattern.coords);\n"
    "\n"
    "  lowp vec3 final = blend(\n"
    "    blend(pattern.pixels.p0, pattern.pixels.p1, weights.x),\n"
    "    blend(pattern.pixels.p2, pattern.pixels.p3, weights.y),\n"
    "    weights.z\n"
    "  );\n"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}";

const std::unordered_map<std::string, std::string> ShaderManager::cut2UpscaleParams = {
    { "USE_DYNAMIC_BLEND", "1" },
    { "BLEND_MIN_CONTRAST_EDGE", "0.25" },
    { "BLEND_MAX_CONTRAST_EDGE", "0.75" },
    { "BLEND_MIN_SHARPNESS", "0.0" },
    { "BLEND_MAX_SHARPNESS", "1.0" },
    { "STATIC_BLEND_SHARPNESS", "1.0" },
    { "EDGE_USE_FAST_LUMA", "0" },
    { "EDGE_MIN_VALUE", "0.03" },
    { "EDGE_MIN_CONTRAST", "2.0" },
    { "LUMA_ADJUST_GAMMA", "0" },
    { "EDGE_SHARPENING", "0" },
    { "EDGE_SHARPENING_AMOUNT", "0.5" },
    { "EDGE_SHARPENING_MIN_VALUE", "0.1" },
    { "EDGE_SHARPENING_THRESHOLD", "0.2" },
};

const std::string ShaderManager::cut2UpscalePass0Vertex =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "attribute vec4 vPosition;\n"
    "attribute vec2 vCoordinate;\n"
    "uniform mediump float screenDensity;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mat4 vViewModel;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float vFlipY;\n"
    "\n"
    "varying HIGHP vec2 c01;\n"
    "varying HIGHP vec2 c02;\n"
    "varying HIGHP vec2 c04;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c07;\n"
    "varying HIGHP vec2 c08;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "varying HIGHP vec2 c11;\n"
    "varying HIGHP vec2 c13;\n"
    "varying HIGHP vec2 c14;\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
    "  HIGHP vec2 screenCoords = coords * textureSize - vec2(0.5);\n"
    "  c01 = (screenCoords + vec2(+0.0, -1.0)) / textureSize;\n"
    "  c02 = (screenCoords + vec2(+1.0, -1.0)) / textureSize;\n"
    "  c04 = (screenCoords + vec2(-1.0, +0.0)) / textureSize;\n"
    "  c05 = (screenCoords + vec2(+0.0, +0.0)) / textureSize;\n"
    "  c06 = (screenCoords + vec2(+1.0, +0.0)) / textureSize;\n"
    "  c07 = (screenCoords + vec2(+2.0, +0.0)) / textureSize;\n"
    "  c08 = (screenCoords + vec2(-1.0, +1.0)) / textureSize;\n"
    "  c09 = (screenCoords + vec2(+0.0, +1.0)) / textureSize;\n"
    "  c10 = (screenCoords + vec2(+1.0, +1.0)) / textureSize;\n"
    "  c11 = (screenCoords + vec2(+2.0, +1.0)) / textureSize;\n"
    "  c13 = (screenCoords + vec2(+0.0, +2.0)) / textureSize;\n"
    "  c14 = (screenCoords + vec2(+1.0, +2.0)) / textureSize;\n"
    "  gl_Position = vPosition;\n"
    "}";

const std::string ShaderManager::cut2UpscalePass0Fragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "#endif\n"
    "\n"
    "#define EPSILON 0.02\n"
    "\n"
    "precision lowp float;\n"
    "\n"
    "uniform lowp sampler2D texture;\n"
    "\n"
    "varying HIGHP vec2 c01;\n"
    "varying HIGHP vec2 c02;\n"
    "varying HIGHP vec2 c04;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c07;\n"
    "varying HIGHP vec2 c08;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "varying HIGHP vec2 c11;\n"
    "varying HIGHP vec2 c13;\n"
    "varying HIGHP vec2 c14;\n"
    "\n"
    "struct Flags {\n"
    "  lowp float type;\n"
    "  bvec3 flip;\n"
    "  lowp vec4 hardEdges;\n"
    "  bool modifier;\n"
    "};\n"
    "\n"
    "lowp float luma(lowp vec3 v) {\n"
    "#if EDGE_USE_FAST_LUMA\n"
    "  lowp float result = v.g;\n"
    "#else\n"
    "  lowp float result = dot(v, vec3(0.299, 0.587, 0.114));\n"
    "#endif\n"
    "#if LUMA_ADJUST_GAMMA\n"
    "  result = sqrt(result);\n"
    "#endif\n"
    "  return result;\n"
    "}\n"
    "\n"
    "lowp float maxOf(lowp float a, lowp float b, lowp float c, lowp float d) {\n"
    "  return max(max(a, b), max(c, d));\n"
    "}\n"
    "\n"
    "lowp float minOf(lowp float a, lowp float b, lowp float c, lowp float d) {\n"
    "  return min(min(a, b), min(c, d));\n"
    "}\n"
    "\n"
    "bvec2 hasDiagonal(lowp float a, lowp float b, lowp float c, lowp float d, lowp float e, lowp float f) {\n"
    "  lowp float dab = distance(a, b);\n"
    "  lowp float dac = distance(a, c);\n"
    "  lowp float dbc = distance(b, c);\n"
    "  lowp float dbd = distance(b, d);\n"
    "  lowp float dcd = distance(c, d);\n"
    "  lowp float dce = distance(c, e);\n"
    "  lowp float ddf = distance(d, f);\n"
    "  lowp float ded = distance(e, d);\n"
    "  lowp float def = distance(e, f);\n"
    "\n"
    "  lowp float leftInnerContrast = maxOf(dac, dce, def, dbd);\n"
    "  lowp float leftOuterContrast = minOf(dab, dcd, ddf, ded);\n"
    "  bool leftCut = max(EDGE_MIN_CONTRAST * leftInnerContrast, EDGE_MIN_VALUE) < leftOuterContrast;\n"
    "\n"
    "  lowp float rightInnerContrast = maxOf(dab, dbd, ddf, dce);\n"
    "  lowp float rightOuterContrast = minOf(dac, dcd, def, dbc);\n"
    "  bool rightCut = max(EDGE_MIN_CONTRAST * rightInnerContrast, EDGE_MIN_VALUE) < rightOuterContrast;\n"
    "\n"
    "  return bvec2(leftCut || rightCut, leftCut);\n"
    "}\n"
    "\n"
    "bool hasDiagonal(lowp float a, lowp float b, lowp float c, lowp float d) {\n"
    "  lowp float diff1 = distance(a, d);\n"
    "  lowp float diff2 = max(\n"
    "    min(distance(a, b), distance(b, d)),\n"
    "    min(distance(a, c), distance(c, d))\n"
    "  );\n"
    "  return max(EDGE_MIN_CONTRAST * diff1, EDGE_MIN_VALUE) < diff2;\n"
    "}\n"
    "\n"
    "lowp float pack(bool a, bool b, bool c, bool d) {\n"
    "  return dot(vec4(float(a), float(b), float(c), float(d)), vec4(2.0, 4.0, 8.0, 16.0)) / 255.0;\n"
    "}\n"
    "\n"
    "mediump float packFloats(lowp vec2 values) {\n"
    "  mediump float result = 0.0;\n"
    "  result += floor(values.x * 2.0 + 0.5) / 3.0;\n"
    "  result += floor(values.y * 2.0 + 0.5) / (3.0 * 3.0);\n"
    "  return result;\n"
    "}\n"
    "\n"
    "lowp vec4 flipEdges(lowp vec4 edges, bvec3 flip) {\n"
    "  lowp vec4 result = edges;\n"
    "\n"
    "  if (flip.x) {\n"
    "    result = vec4(-result.x, result.w, -result.z, result.y);\n"
    "  }\n"
    "\n"
    "  if (flip.y) {\n"
    "    result = vec4(result.z, -result.y, result.x, -result.w);\n"
    "  }\n"
    "\n"
    "  if (flip.z) {\n"
    "    result = vec4(result.w, result.z, result.y, result.x);\n"
    "  }\n"
    "\n"
    "  return result;\n"
    "}\n"
    "\n"
    "lowp vec4 mergeEdges(lowp vec4 softEdges, lowp vec4 hardEdges) {\n"
    "  lowp vec4 result = softEdges;\n"
    "\n"
    "  result.x = (hardEdges.x > -2.0) ? hardEdges.x : result.x;\n"
    "  result.y = (hardEdges.y > -2.0) ? hardEdges.y : result.y;\n"
    "  result.z = (hardEdges.z > -2.0) ? hardEdges.z : result.z;\n"
    "  result.w = (hardEdges.w > -2.0) ? hardEdges.w : result.w;\n"
    "\n"
    "  return result;\n"
    "}\n"
    "\n"
    "float softEdgeWeight(lowp float l0, lowp float l1, lowp float l2, lowp float l3) {\n"
    "  lowp float maxRange = maxOf(l0, l1, l2, l3);\n"
    "  lowp float minRange = minOf(l0, l1, l2, l3);\n"
    "  lowp float threshold = max(EDGE_SHARPENING_MIN_VALUE, EDGE_SHARPENING_THRESHOLD * (maxRange - minRange));\n"
    "  return step(threshold, abs(l3 - l1)) - step(threshold, abs(l2 - l0));\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 t01 = texture2D(texture, c01).rgb;\n"
    "  lowp vec3 t02 = texture2D(texture, c02).rgb;\n"
    "  lowp vec3 t04 = texture2D(texture, c04).rgb;\n"
    "  lowp vec3 t05 = texture2D(texture, c05).rgb;\n"
    "  lowp vec3 t06 = texture2D(texture, c06).rgb;\n"
    "  lowp vec3 t07 = texture2D(texture, c07).rgb;\n"
    "  lowp vec3 t08 = texture2D(texture, c08).rgb;\n"
    "  lowp vec3 t09 = texture2D(texture, c09).rgb;\n"
    "  lowp vec3 t10 = texture2D(texture, c10).rgb;\n"
    "  lowp vec3 t11 = texture2D(texture, c11).rgb;\n"
    "  lowp vec3 t13 = texture2D(texture, c13).rgb;\n"
    "  lowp vec3 t14 = texture2D(texture, c14).rgb;\n"
    "\n"
    "  lowp float l01 = luma(t01);\n"
    "  lowp float l02 = luma(t02);\n"
    "  lowp float l04 = luma(t04);\n"
    "  lowp float l05 = luma(t05);\n"
    "  lowp float l06 = luma(t06);\n"
    "  lowp float l07 = luma(t07);\n"
    "  lowp float l08 = luma(t08);\n"
    "  lowp float l09 = luma(t09);\n"
    "  lowp float l10 = luma(t10);\n"
    "  lowp float l11 = luma(t11);\n"
    "  lowp float l13 = luma(t13);\n"
    "  lowp float l14 = luma(t14);\n"
    "\n"
    "  // Main diagonals\n"
    "  bool d05_10 = hasDiagonal(l05, l06, l09, l10);\n"
    "  bool d06_09 = hasDiagonal(l06, l05, l10, l09);\n"
    "\n"
    "  // Saddle fix\n"
    "  if (d05_10 && d06_09) {\n"
    "    lowp float diff1 = 4.0 * distance(l05, l10) + distance(l06, l01) + distance(l11, l06) + distance(l09, l04) + distance(l14, l09);\n"
    "    lowp float diff2 = 4.0 * distance(l06, l09) + distance(l05, l02) + distance(l08, l05) + distance(l10, l07) + distance(l13, l10);\n"
    "    d05_10 = diff1 + EPSILON < diff2;\n"
    "    d06_09 = diff2 + EPSILON < diff1;\n"
    "  }\n"
    "\n"
    "  // Vertical diagonals\n"
    "  bvec2 d01_10 = hasDiagonal(l10, l09, l06, l05, l02, l01);\n"
    "  bvec2 d02_09 = hasDiagonal(l09, l10, l05, l06, l01, l02);\n"
    "  bvec2 d05_14 = hasDiagonal(l05, l06, l09, l10, l13, l14);\n"
    "  bvec2 d06_13 = hasDiagonal(l06, l05, l10, l09, l14, l13);\n"
    "\n"
    "  // Horizontal diagonals\n"
    "  bvec2 d04_10 = hasDiagonal(l10, l06, l09, l05, l08, l04);\n"
    "  bvec2 d06_08 = hasDiagonal(l06, l10, l05, l09, l04, l08);\n"
    "  bvec2 d05_11 = hasDiagonal(l05, l09, l06, l10, l07, l11);\n"
    "  bvec2 d07_09 = hasDiagonal(l09, l05, l10, l06, l11, l07);\n"
    "\n"
    "  bvec4 type5 = bvec4(d02_09.x && d06_08.x, d01_10.x && d05_11.x, d06_13.x && d07_09.x, d05_14.x && d04_10.x);\n"
    "  bvec4 type4 = bvec4(d05_11.x && d06_08.x, d04_10.x && d07_09.x, d05_14.x && d02_09.x, d01_10.x && d06_13.x);\n"
    "  bvec4 type3 = bvec4(d05_11.x && d04_10.x, d06_08.x && d07_09.x, d01_10.x && d05_14.x, d02_09.x && d06_13.x);\n"
    "  bvec4 type2_v = bvec4(d01_10.x, d02_09.x, d05_14.x, d06_13.x);\n"
    "  bvec4 type2_h = bvec4(d04_10.x, d06_08.x, d05_11.x, d07_09.x);\n"
    "  bvec2 type1 = bvec2(d05_10, d06_09);\n"
    "\n"
    "  bool bottomCut = any(bvec4(all(d05_11), all(d07_09), all(d05_14), all(d06_13)));\n"
    "  bool topCut = any(bvec4(all(d01_10), all(d02_09), all(d04_10), all(d06_08)));\n"
    "\n"
    "  lowp vec4 final = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "\n"
    "  Flags flags;\n"
    "  flags.type = 0.0;\n"
    "  flags.flip = bvec3(false);\n"
    "  flags.modifier = false;\n"
    "\n"
    "  #if EDGE_SHARPENING\n"
    "  flags.hardEdges = vec4(-4.0);\n"
    "  #else\n"
    "  flags.hardEdges = vec4(0.0);\n"
    "  #endif\n"
    "\n"
    "  if (any(type5)) {\n"
    "    flags.type = 0.15;\n"
    "    flags.flip = bvec3(type5.z, type5.x, type5.y);\n"
    "    flags.hardEdges.z = -1.0;\n"
    "    flags.hardEdges.w = 1.0;\n"
    "    flags.modifier = true;\n"
    "  } else if (any(type4)) {\n"
    "    flags.type = 0.0;\n"
    "    flags.flip = bvec3(type4.w, type4.y, type4.x || type4.y);\n"
    "    flags.hardEdges.x = topCut ? 1.0 : -1.0;\n"
    "    flags.hardEdges.z = bottomCut ? 1.0 : -1.0;\n"
    "    flags.modifier = true;\n"
    "  } else if (any(type3)) {\n"
    "    flags.type = 0.35;\n"
    "    flags.flip = bvec3(type3.w, type3.y, type3.x || type3.y);\n"
    "    flags.hardEdges.x = topCut ? -1.0 : 1.0;\n"
    "    flags.hardEdges.z = bottomCut ? 1.0 : -1.0;\n"
    "  } else if (any(type2_v)) {\n"
    "    flags.type = 0.25;\n"
    "    flags.flip = bvec3(type2_v.x || type2_v.w, type2_v.y || type2_v.x, false);\n"
    "    flags.hardEdges.z = (bottomCut || topCut) ? 1.0 : -1.0;\n"
    "  } else if (any(type2_h)) {\n"
    "    flags.type = 0.25;\n"
    "    flags.flip = bvec3(type2_h.y || type2_h.x, type2_h.w || type2_h.x, true);\n"
    "    flags.hardEdges.z = (bottomCut || topCut) ? 1.0 : -1.0;\n"
    "  } else if (any(type1)) {\n"
    "    flags.type = 0.15;\n"
    "    flags.flip = bvec3(type1.y, false, false);\n"
    "  }\n"
    "\n"
    "  lowp vec4 edges;\n"
    "\n"
    "#if EDGE_SHARPENING\n"
    "  lowp vec4 softEdges = vec4(\n"
    "    softEdgeWeight(l04, l05, l06, l07),\n"
    "    softEdgeWeight(l02, l06, l10, l14),\n"
    "    softEdgeWeight(l08, l09, l10, l11),\n"
    "    softEdgeWeight(l01, l05, l09, l13)\n"
    "  );\n"
    "\n"
    "  softEdges = flipEdges(softEdges, flags.flip);\n"
    "  edges = mergeEdges(softEdges, flags.hardEdges);\n"
    "#else\n"
    "  edges = flags.hardEdges;\n"
    "#endif\n"
    "\n"
    "  gl_FragColor = vec4(\n"
    "    flags.type,\n"
    "    pack(flags.flip.x, flags.flip.y, flags.flip.z, flags.modifier),\n"
    "    packFloats((edges.xy + 1.0) * 0.5),\n"
    "    packFloats((edges.zw + 1.0) * 0.5)\n"
    "  );\n"
    "}";

const std::string ShaderManager::cut2UpscalePass1Vertex =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "precision mediump float;\n"
    "#endif\n"
    "\n"
    "attribute vec4 vPosition;\n"
    "attribute vec2 vCoordinate;\n"
    "uniform mediump float screenDensity;\n"
    "uniform lowp sampler2D texture;\n"
    "uniform mat4 vViewModel;\n"
    "uniform HIGHP vec2 textureSize;\n"
    "uniform mediump float vFlipY;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying HIGHP vec2 passCoords;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001;\n"
//    "  HIGHP vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.0001 * 0.25 + vec2(0.20, 0.0);\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  c05 = (screenCoords + vec2(+0.0, +0.0)) / textureSize;\n"
    "  c06 = (screenCoords + vec2(+1.0, +0.0)) / textureSize;\n"
    "  c09 = (screenCoords + vec2(+0.0, +1.0)) / textureSize;\n"
    "  c10 = (screenCoords + vec2(+1.0, +1.0)) / textureSize;\n"
    "  passCoords = vec2(c05.x, mix(c05.y, 1.0 - c05.y, vFlipY));\n"
    "  gl_Position = vViewModel * vPosition;\n"
    "}";

const std::string ShaderManager::cut2UpscalePass1Fragment =
    "#ifdef GL_FRAGMENT_PRECISION_HIGH\n"
    "#define HIGHP highp\n"
    "#else\n"
    "#define HIGHP mediump\n"
    "#endif\n"
    "#define EPSILON 0.02\n"
    "\n"
    "precision lowp float;\n"
    "\n"
    "uniform lowp sampler2D texture;\n"
    "uniform lowp sampler2D previousPass;\n"
    "\n"
    "varying HIGHP vec2 screenCoords;\n"
    "varying HIGHP vec2 coords;\n"
    "varying HIGHP vec2 passCoords;\n"
    "varying HIGHP vec2 c05;\n"
    "varying HIGHP vec2 c06;\n"
    "varying HIGHP vec2 c09;\n"
    "varying HIGHP vec2 c10;\n"
    "\n"
    "lowp float luma(lowp vec3 v) {\n"
    "  return v.g;\n"
    "}\n"
    "\n"
    "struct Pixels {\n"
    "  lowp vec3 p0;\n"
    "  lowp vec3 p1;\n"
    "  lowp vec3 p2;\n"
    "  lowp vec3 p3;\n"
    "};\n"
    "\n"
    "struct Pattern {\n"
    "  Pixels pixels;\n"
    "  lowp vec3 weights;\n"
    "  lowp vec3 edgeWeights;\n"
    "  lowp vec3 softEdgeWeights;\n"
    "};\n"
    "\n"
    "struct Flags {\n"
    "  int type;\n"
    "  bvec3 flip;\n"
    "  lowp float modifier;\n"
    "  lowp vec4 edgeWeight;\n"
    "};\n"
    "\n"
    "bvec4 unpackBools(lowp float values) {\n"
    "  lowp vec4 modulos = mod(values * vec4(0.5, 0.25, 0.125, 0.0625), vec4(2.0));\n"
    "  return greaterThan(floor(modulos), vec4(0.5));\n"
    "}\n"
    "\n"
    "lowp vec2 unpackFloats(lowp float value) {\n"
    "  lowp float val1 = floor(value * 3.0) / 2.0;\n"
    "  lowp float val2 = floor(fract(value * 3.0) * 3.0) / 2.0;\n"
    "  return vec2(val1, val2);\n"
    "}\n"
    "\n"
    "lowp vec2 round(lowp vec2 v) {\n"
    "  return floor(v + vec2(0.5));\n"
    "}\n"
    "\n"
    "Flags parseFlags(lowp vec4 flagsPixel) {\n"
    "  bvec4 flagBits = unpackBools(floor(flagsPixel.y * 255.0 + 0.5));\n"
    "  lowp vec2 edgeWeights0 = round(unpackFloats(flagsPixel.z) * 2.0 - 1.0);\n"
    "  lowp vec2 edgeWeights1 = round(unpackFloats(flagsPixel.w) * 2.0 - 1.0);\n"
    "\n"
    "  Flags flags;\n"
    "  flags.type = int(flagsPixel.x * 10.0);\n"
    "  flags.flip = flagBits.rgb;\n"
    "  flags.modifier = float(flagBits.a);\n"
    "  flags.edgeWeight = vec4(edgeWeights0, edgeWeights1);\n"
    "\n"
    "  return flags;\n"
    "}\n"
    "\n"
    "lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {\n"
    "  return clamp((t - edge0) / (edge1 - edge0 + EPSILON), 0.0, 1.0);\n"
    "}\n"
    "\n"
    "lowp float sharpSmooth(lowp float t, lowp float sharpness) {\n"
    "  return linearStep(sharpness, 1.0 - sharpness, t);\n"
    "}\n"
    "\n"
    "lowp float sharpness(lowp float l1, lowp float l2) {\n"
    "#if USE_DYNAMIC_BLEND\n"
    "  lowp float lumaDiff = abs(l1 - l2);\n"
    "  lowp float contrast = linearStep(BLEND_MIN_CONTRAST_EDGE, BLEND_MAX_CONTRAST_EDGE, lumaDiff);\n"
    "  lowp float result = mix(BLEND_MIN_SHARPNESS * 0.5, BLEND_MAX_SHARPNESS * 0.5, contrast);\n"
    "#else\n"
    "  lowp float result = STATIC_BLEND_SHARPNESS * 0.5;\n"
    "#endif\n"
    "  return result;\n"
    "}\n"
    "\n"
    "lowp vec3 blend(lowp vec3 a, lowp vec3 b, lowp float hardEdge, lowp float softEdge, lowp float t) {\n"
    "  lowp float threshold = 0.5 * hardEdge;\n"
    "#if EDGE_SHARPENING\n"
    "  threshold += (1.0 - distance(luma(a), luma(b))) * softEdge * EDGE_SHARPENING_AMOUNT;\n"
    "#endif\n"
    "  lowp float sharpness = sharpness(luma(a), luma(b)) * (1.0 - abs(threshold));\n"
    "  lowp float nt = linearStep(\n"
    "    max(threshold, 0.0) + sharpness,\n"
    "    min(1.0 + threshold, 1.0) - sharpness,\n"
    "    t\n"
    "  );\n"
    "  return mix(a, b, nt);\n"
    "}\n"
    "\n"
    "Pattern pattern0(Pixels pixels, lowp float modifier, lowp vec4 edgeWeights, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "  result.pixels = pixels;\n"
    "  result.weights = vec3(pxCoords.x, pxCoords.x, pxCoords.y);\n"
    "  result.edgeWeights = vec3(\n"
    "    edgeWeights.x * modifier,\n"
    "    edgeWeights.z * modifier,\n"
    "    0.0\n"
    "  );\n"
    "  result.softEdgeWeights = vec3(\n"
    "    edgeWeights.x * (1.0 - modifier),\n"
    "    edgeWeights.z * (1.0 - modifier),\n"
    "    mix(edgeWeights.w, edgeWeights.y, pxCoords.x)\n"
    "  );\n"
    "  return result;\n"
    "}\n"
    "\n"
    "Pattern pattern1(Pixels pixels, lowp float modifier, lowp vec4 edgeWeights, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "  if (pxCoords.y > pxCoords.x) {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p2, pixels.p2, pixels.p3);\n"
    "    result.weights.x = pxCoords.y - pxCoords.x;\n"
    "    result.weights.y = 1.0 - result.weights.x;\n"
    "    result.weights.z = pxCoords.x / (result.weights.y + EPSILON);\n"
    "    result.edgeWeights = vec3(edgeWeights.w * modifier, edgeWeights.z * modifier, 0.0);\n"
    "    result.softEdgeWeights = vec3(edgeWeights.w * (1.0 - modifier), edgeWeights.z * (1.0 - modifier), 0.0);\n"
    "  } else {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p1, pixels.p1, pixels.p3);\n"
    "    result.weights.x = pxCoords.x - pxCoords.y;\n"
    "    result.weights.y = 1.0 - result.weights.x;\n"
    "    result.weights.z = pxCoords.y / (result.weights.y + EPSILON);\n"
    "    result.softEdgeWeights = vec3(edgeWeights.x, edgeWeights.y, 0.0);\n"
    "  }\n"
    "  return result;\n"
    "}\n"
    "\n"
    "Pattern pattern2(Pixels pixels, lowp float modifier, lowp vec4 edgeWeights, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "  if (pxCoords.y > 2.0 * pxCoords.x) {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p2, pixels.p2, pixels.p3);\n"
    "    result.weights.x = pxCoords.y - 2.0 * pxCoords.x;\n"
    "    result.weights.y = 0.5 * (1.0 - result.weights.x);\n"
    "    result.weights.z = pxCoords.x / (result.weights.y + EPSILON);\n"
    "    result.edgeWeights = vec3(0.0, edgeWeights.z, 0.0);\n"
    "    result.softEdgeWeights = vec3(edgeWeights.w, 0.0, mix(edgeWeights.w, edgeWeights.y, pxCoords.x));\n"
    "  } else {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p1, pixels.p2, pixels.p3);\n"
    "    result.weights.x = (pxCoords.x - 0.5 * pxCoords.y) / (1.0 - 0.5 * pxCoords.y + EPSILON);\n"
    "    result.weights.y = 0.5 + 0.5 * result.weights.x;\n"
    "    result.weights.z = pxCoords.y;\n"
    "    result.edgeWeights = vec3(0.0, edgeWeights.z, 0.0);\n"
    "    result.softEdgeWeights = vec3(edgeWeights.x, 0.0, mix(edgeWeights.w, edgeWeights.y, pxCoords.x));\n"
    "  }\n"
    "  return result;\n"
    "}\n"
    "\n"
    "Pattern pattern3(Pixels pixels, lowp float modifier, lowp vec4 edgeWeights, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "  if (pxCoords.y > 2.0 * pxCoords.x) {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p2, pixels.p2, pixels.p3);\n"
    "    result.weights.x = pxCoords.y - 2.0 * pxCoords.x;\n"
    "    result.weights.y = 0.5 * (1.0 - result.weights.x);\n"
    "    result.weights.z = pxCoords.x / (result.weights.y + EPSILON);\n"
    "    result.edgeWeights = vec3(0.0, edgeWeights.z, 0.0);\n"
    "    result.softEdgeWeights = vec3(edgeWeights.w, 0.0, mix(edgeWeights.w, edgeWeights.y, pxCoords.x));\n"
    "  } else if (pxCoords.y < 2.0 * pxCoords.x - 1.0) {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p1, pixels.p1, pixels.p3);\n"
    "    result.weights.x = pxCoords.x - 0.5 * pxCoords.y;\n"
    "    result.weights.y = pxCoords.y + 2.0 * (1.0 - pxCoords.x);\n"
    "    result.weights.z = pxCoords.y / (result.weights.y + EPSILON);\n"
    "    result.edgeWeights = vec3(edgeWeights.x, 0.0, 0.0);\n"
    "    result.softEdgeWeights = vec3(0.0, edgeWeights.y, mix(edgeWeights.w, edgeWeights.y, pxCoords.x));\n"
    "  } else {\n"
    "    result.pixels = Pixels(pixels.p0, pixels.p1, pixels.p2, pixels.p3);\n"
    "    result.weights.x = pxCoords.x - 0.5 * pxCoords.y;\n"
    "    result.weights.y = pxCoords.x + 0.5 * (1.0 - pxCoords.y);\n"
    "    result.weights.z = pxCoords.y;\n"
    "    result.edgeWeights = vec3(edgeWeights.x, edgeWeights.z, 0.0);\n"
    "    result.softEdgeWeights = vec3(0.0, 0.0, mix(edgeWeights.w, edgeWeights.y, pxCoords.x));\n"
    "  }\n"
    "  return result;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 t05 = texture2D(texture, c05).rgb;\n"
    "  lowp vec3 t06 = texture2D(texture, c06).rgb;\n"
    "  lowp vec3 t09 = texture2D(texture, c09).rgb;\n"
    "  lowp vec3 t10 = texture2D(texture, c10).rgb;\n"
    "\n"
    "  lowp vec4 flagsPixel = texture2D(previousPass, passCoords);\n"
    "  Flags flags = parseFlags(flagsPixel);\n"
    "  Pixels pixels = Pixels(t05, t06, t09, t10);\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  if (flags.flip.x) {\n"
    "    pixels = Pixels(pixels.p1, pixels.p0, pixels.p3, pixels.p2);\n"
    "    pxCoords.x = 1.0 - pxCoords.x;\n"
    "  }\n"
    "\n"
    "  if (flags.flip.y) {\n"
    "    pixels = Pixels(pixels.p2, pixels.p3, pixels.p0, pixels.p1);\n"
    "    pxCoords.y = 1.0 - pxCoords.y;\n"
    "  }\n"
    "\n"
    "  if (flags.flip.z) {\n"
    "    pixels = Pixels(pixels.p0, pixels.p2, pixels.p1, pixels.p3);\n"
    "    pxCoords = pxCoords.yx;\n"
    "  }\n"
    "\n"
    "  Pattern pattern;\n"
    "\n"
    "  if (flags.type == 0) {\n"
    "    pattern = pattern0(pixels, flags.modifier, flags.edgeWeight, pxCoords);\n"
    "    lowp vec3 c = vec3(1.0, flags.modifier, 0.0);\n"
//    "    pattern.pixels = Pixels(mix(pattern.pixels.p0, c, 0.125), mix(pattern.pixels.p1, c, 0.125), mix(pattern.pixels.p2, c, 0.125), mix(pattern.pixels.p3, c, 0.125));\n"
    "  } else if (flags.type == 1) {\n"
    "    pattern = pattern1(pixels, flags.modifier, flags.edgeWeight, pxCoords);\n"
    "    lowp vec3 c = vec3(0.0, 1.0, 0.0);\n"
//    "    pattern.pixels = Pixels(mix(pattern.pixels.p0, c, 0.125), mix(pattern.pixels.p1, c, 0.125), mix(pattern.pixels.p2, c, 0.125), mix(pattern.pixels.p3, c, 0.125));\n"
    "  } else if (flags.type == 2) {\n"
    "    pattern = pattern2(pixels, flags.modifier, flags.edgeWeight, pxCoords);\n"
    "    lowp vec3 c = vec3(0.0, 0.0, 1.0);\n"
//    "    pattern.pixels = Pixels(mix(pattern.pixels.p0, c, 0.125), mix(pattern.pixels.p1, c, 0.125), mix(pattern.pixels.p2, c, 0.125), mix(pattern.pixels.p3, c, 0.125));\n"
    "  } else {\n"
    "    pattern = pattern3(pixels, flags.modifier, flags.edgeWeight, pxCoords);\n"
    "    lowp vec3 c = vec3(1.0, 0.0, 1.0);\n"
//    "    pattern.pixels = Pixels(mix(pattern.pixels.p0, c, 0.125), mix(pattern.pixels.p1, c, 0.125), mix(pattern.pixels.p2, c, 0.125), mix(pattern.pixels.p3, c, 0.125));\n"
    "  }\n"
    "\n"
    "  lowp vec3 final = blend(\n"
    "    blend(pattern.pixels.p0, pattern.pixels.p1, pattern.edgeWeights.x, pattern.softEdgeWeights.x, pattern.weights.x),\n"
    "    blend(pattern.pixels.p2, pattern.pixels.p3, pattern.edgeWeights.y, pattern.softEdgeWeights.y, pattern.weights.y),\n"
    "    pattern.edgeWeights.z,\n"
    "    pattern.softEdgeWeights.z,\n"
    "    pattern.weights.z\n"
    "  );\n"
    "\n"
    "  gl_FragColor = vec4(final.rgb, 1.0);\n"
    "}";

ShaderManager::Chain ShaderManager::getShader(const ShaderManager::Config& config) {
    switch (config.type) {
    case Type::SHADER_DEFAULT: {
        return { { { defaultShaderVertex, defaultShaderFragment, true, 1.0 } }, true };
    }

    case Type::SHADER_CRT: {
        return { { { defaultShaderVertex, crtShaderFragment, true, 1.0 } } , true };
    }

    case Type::SHADER_LCD: {
        return { { {defaultShaderVertex, lcdShaderFragment, true, 1.0 } }, true };
    }

    case Type::SHADER_SHARP: {
        return { { { defaultShaderVertex, defaultSharpFragment, true, 1.0 } }, true };
    }

    case Type::SHADER_UPSCALE_CUT: {
        std::string defines = buildDefines(cutUpscaleParams, config.params);
        return { { {
            defines + cutUpscaleVertex,
            defines + cutUpscaleFragment,
            false,
            1.0
        } } , false };
    }

    case Type::SHADER_UPSCALE_CUT2: {
        std::string defines = buildDefines(cut2UpscaleParams, config.params);
        return {
            {
                {
                    defines + cut2UpscalePass0Vertex,
                    defines + cut2UpscalePass0Fragment,
                    false,
                    1.0
                },
                {
                    defines + cut2UpscalePass1Vertex,
                    defines + cut2UpscalePass1Fragment,
                    false,
                    1.0
                }
            },
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

bool ShaderManager::Chain::operator==(const ShaderManager::Chain &other) const {
    return this->passes == other.passes && this->linearTexture == other.linearTexture;
}

bool ShaderManager::Chain::operator!=(const ShaderManager::Chain &other) const {
    return !(*this == other);
}

bool ShaderManager::Pass::operator==(const ShaderManager::Pass &other) const {
    return this->linear == other.linear && this->scale == other.scale &&
           this->vertex == other.vertex && this->fragment == other.fragment;
}
} //namespace libretrodroid
