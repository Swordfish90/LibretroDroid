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
    { "REDUCE_ANTI_ALIASING", "0" },
    { "REDUCE_ANTI_ALIASING_AMOUNT", "0.5" },
    { "REDUCE_ANTI_ALIASING_MIN_CONTRAST_EDGE", "0.03" },
    { "REDUCE_ANTI_ALIASING_CONTRAST_THRESHOLD", "0.98" },
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
    "#define Range lowp vec2\n"
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
    "lowp float quickPackBools2(bvec2 values) {\n"
    "  return dot(vec2(values), vec2(0.5, 0.25));\n"
    "}\n"
    "\n"
    "lowp float quickPackFloats2(lowp vec2 values) {\n"
    "  return dot(floor(values * vec2(14.0) + vec2(0.5)), vec2(0.0625, 0.00390625));\n"
    "}\n"
    "\n"
    "lowp float hardEdgeWeights(lowp vec2 gn, lowp vec2 gs) {\n"
    "  lowp float result = 0.0;\n"
    "  result += step(2.0 * gn.y, gn.x) - step(2.0 * gn.x, gn.y);\n"
    "  result += step(2.0 * gs.x, gs.y) - step(2.0 * gs.y, gs.x);\n"
    "  return step(abs(result), 1.5) * clamp(result, -1.0, 1.0);\n"
    "}\n"
    "\n"
    "lowp float softEdgeWeight(lowp float a, lowp float b, lowp float c, lowp float d) {\n"
    "  lowp vec4 ws = vec4(abs(d - b), abs(c - d), abs(a - c), abs(b - a));\n"
    "  ws = normalize(ws + vec4(EPSILON));\n"
    "  lowp float result = ws.x * ws.y - ws.z * ws.w;\n"
    "  return clamp(4.0 * result, -1.0, 1.0);\n"
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
    "  lowp vec2 gn = abs(vec2(l01 - l06, l02 - l05));\n"
    "  lowp vec2 ge = abs(vec2(l06 - l11, l07 - l10));\n"
    "  lowp vec2 gs = abs(vec2(l09 - l14, l10 - l13));\n"
    "  lowp vec2 gw = abs(vec2(l04 - l09, l05 - l08));\n"
    "  lowp vec2 gc = abs(vec2(l05 - l10, l06 - l09));\n"
    "\n"
    "  lowp vec4 edges = vec4(0.5);\n"
    "  edges.x = hardEdgeWeights(gn.xy, gc.xy);\n"
    "  edges.y = hardEdgeWeights(ge.yx, gc.yx);\n"
    "  edges.z = hardEdgeWeights(gc.xy, gs.xy);\n"
    "  edges.w = hardEdgeWeights(gc.yx, gw.yx);\n"
    "\n"
    "  lowp vec4 ec = abs(vec4(l06, l10, l09, l05) - vec4(l05, l06, l10, l09));\n"
    "  lowp vec2 cg = 0.25 * (gn + ge + gs + gw);\n"
    "\n"
    "  lowp float oppositeEdgesContrast = max(ec.x + ec.z, ec.y + ec.w) - EPSILON;\n"
    "  bool allowLeftTriangle = max(ec.w + ec.z, ec.x + ec.y) >= oppositeEdgesContrast;\n"
    "  bool allowRightTriangle = max(ec.w + ec.x, ec.z + ec.y) >= oppositeEdgesContrast;\n"
    "  bool rightTriangle = 2.0 * gc.x + cg.x + EPSILON < gc.y + cg.y && allowLeftTriangle;\n"
    "  bool leftTriangle = 2.0 * gc.y + cg.y + EPSILON < gc.x + cg.x && allowRightTriangle;\n"
    "\n"
    "  bvec2 flags = bvec2(leftTriangle || rightTriangle, leftTriangle);\n"
    "\n"
    "#if REDUCE_ANTI_ALIASING\n"
    "  lowp vec4 softEdges = REDUCE_ANTI_ALIASING_AMOUNT * vec4(\n"
    "    softEdgeWeight(l04, l05, l06, l07),\n"
    "    softEdgeWeight(l02, l06, l10, l14),\n"
    "    softEdgeWeight(l08, l09, l10, l11),\n"
    "    softEdgeWeight(l01, l05, l09, l13)\n"
    "  );\n"
    "\n"
    "  edges = clamp(edges + softEdges, vec4(-1.0), vec4(1.0));\n"
    "#endif\n"
    "\n"
    "  if (flags.y) {\n"
    "    edges = vec4(-edges.x, edges.w, -edges.z, edges.y);\n"
    "  }\n"
    "\n"
    "  gl_FragColor = vec4(\n"
    "    quickPackBools2(flags),\n"
    "    quickPackFloats2(edges.xy * 0.5 + vec2(0.5)),\n"
    "    quickPackFloats2(edges.zw * 0.5 + vec2(0.5)),\n"
    "    1.0\n"
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
    "};\n"
    "\n"
    "struct Flags {\n"
    "  bool flip;\n"
    "  bool triangle;\n"
    "  lowp vec4 edgeWeight;\n"
    "};\n"
    "\n"
    "lowp vec2 quickUnpackFloats2(lowp float value) {\n"
    "  lowp vec2 result = vec2(0.0);\n"
    "  lowp float current = value;\n"
    "\n"
    "  current *= 16.0;\n"
    "  result.x = floor(current);\n"
    "  current -= result.x;\n"
    "\n"
    "  current *= 16.0;\n"
    "  result.y = floor(current);\n"
    "  current -= result.y;\n"
    "\n"
    "  return result / 14.0;\n"
    "}\n"
    "\n"
    "bvec2 quickUnpackBools2(lowp float value) {\n"
    "  lowp vec2 result = vec2(0.0);\n"
    "  lowp float current = value;\n"
    "\n"
    "  current *= 2.0;\n"
    "  result.x = floor(current);\n"
    "  current -= result.x;\n"
    "\n"
    "  current *= 2.0;\n"
    "  result.y = floor(current);\n"
    "  current -= result.y;\n"
    "\n"
    "  return greaterThan(result, vec2(0.5));\n"
    "}\n"
    "\n"
    "Flags parseFlags(lowp vec3 flagsPixel) {\n"
    "\n"
    "  Flags flags;\n"
    "  flags.edgeWeight = vec4(0.5) + 0.5 * vec4(\n"
    "    quickUnpackFloats2(flagsPixel.y + 0.001953125) - vec2(0.5),\n"
    "    quickUnpackFloats2(flagsPixel.z + 0.001953125) - vec2(0.5)\n"
    "  );\n"
    "  bvec2 boolFlags = quickUnpackBools2(flagsPixel.x + 0.125);\n"
    "  flags.triangle = boolFlags.x;\n"
    "  flags.flip = boolFlags.y;\n"
    "  return flags;\n"
    "}\n"
    "\n"
    "lowp float linearStep(lowp float edge0, lowp float edge1, lowp float t) {\n"
    "  return clamp((t - edge0) / (edge1 - edge0), 0.0, 1.0);\n"
    "}\n"
    "\n"
    "lowp float sharpness(lowp float l1, lowp float l2) {\n"
    "#if USE_DYNAMIC_BLEND\n"
    "  const lowp float blendDiffInv = 1.0 / (BLEND_MAX_CONTRAST_EDGE - BLEND_MIN_CONTRAST_EDGE);\n"
    "  lowp float lumaDiff = abs(l1 - l2);\n"
    "  lowp float contrast = clamp((lumaDiff - BLEND_MIN_CONTRAST_EDGE) * blendDiffInv, 0.0, 1.0);\n"
    "  lowp float result = mix(BLEND_MIN_SHARPNESS * 0.5, BLEND_MAX_SHARPNESS * 0.5, contrast);\n"
    "#else\n"
    "  lowp float result = STATIC_BLEND_SHARPNESS * 0.5;\n"
    "#endif\n"
    "  return result;\n"
    "}\n"
    "\n"
    "lowp vec3 blend(lowp vec3 a, lowp vec3 b, lowp float t) {\n"
    "  lowp float sharpness = sharpness(luma(a), luma(b));\n"
    "  lowp float nt = clamp((t - sharpness) / (1.0 - 2.0 * sharpness + EPSILON), 0.0 , 1.0);\n"
    "  return mix(a, b, nt);\n"
    "}\n"
    "\n"
    "lowp float adjustMidpoint(lowp float x, lowp float midPoint) {\n"
    "  lowp float result = 0.0;\n"
    "  result += clamp(x / midPoint, 0.0, 1.0);\n"
    "  result += clamp((x - midPoint) / (1.0 - midPoint), 0.0, 1.0);\n"
    "  return 0.5 * result;\n"
    "}\n"
    "\n"
    "lowp vec3 triangleWeights(lowp vec2 pxCoords, lowp vec2 edgeWeights) {\n"
    "  lowp vec3 weights = vec3(0.0);\n"
    "  lowp float m = edgeWeights.x;\n"
    "  lowp float n = edgeWeights.y;\n"
    "  lowp float a = (n * m + pxCoords.y * (1.0 - m - n)) / (n * m + pxCoords.x * (1.0 - m - n));\n"
    "  lowp vec2 projections = vec2((pxCoords.y -a * pxCoords.x), (1.0 - pxCoords.y) / a + pxCoords.x);\n"
    "  return vec3(\n"
    "    adjustMidpoint(projections.x, m),\n"
    "    adjustMidpoint(projections.y, n),\n"
    "    pxCoords.x / (projections.y + EPSILON)\n"
    "  );\n"
    "}"
    "\n"
    "lowp vec3 quadWeights(lowp vec2 pxCoords, lowp vec4 edgeWeights) {\n"
    "  lowp vec2 splits = vec2(\n"
    "    mix(edgeWeights.x, edgeWeights.z, pxCoords.y),\n"
    "    mix(edgeWeights.w, edgeWeights.y, pxCoords.x)\n"
    "  );\n"
    "  return vec3(\n"
    "    adjustMidpoint(pxCoords.x, splits.x),\n"
    "    adjustMidpoint(pxCoords.x, splits.x),\n"
    "    adjustMidpoint(pxCoords.y, splits.y)\n"
    "  );\n"
    "}\n"
    "\n"
    "Pattern pattern(Pixels pixels, lowp vec4 edgeWeights, bool triangle, lowp vec2 pxCoords) {\n"
    "  Pattern result;\n"
    "\n"
    "  bool firstTriangle = triangle && pxCoords.x <= pxCoords.y;\n"
    "  bool secondTriangle = triangle && pxCoords.x >= 1.0 - (1.0 - pxCoords.y);\n"
    "\n"
    "  result.weights = triangle\n"
    "    ? triangleWeights(firstTriangle ? pxCoords : pxCoords.yx, firstTriangle ? edgeWeights.wz : edgeWeights.xy)\n"
    "    : quadWeights(pxCoords, edgeWeights);\n"
    "\n"
    "  result.pixels = Pixels(\n"
    "    pixels.p0,\n"
    "    firstTriangle ? pixels.p2 : pixels.p1,\n"
    "    secondTriangle ? pixels.p1 : pixels.p2,\n"
    "    pixels.p3\n"
    "  );\n"
    "\n"
    "  return result;\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  lowp vec3 t05 = texture2D(texture, c05).rgb;\n"
    "  lowp vec3 t06 = texture2D(texture, c06).rgb;\n"
    "  lowp vec3 t09 = texture2D(texture, c09).rgb;\n"
    "  lowp vec3 t10 = texture2D(texture, c10).rgb;\n"
    "\n"
    "  lowp vec3 flagsPixel = texture2D(previousPass, passCoords).xyz;\n"
    "  Flags flags = parseFlags(flagsPixel);\n"
    "  Pixels pixels = Pixels(t05, t06, t09, t10);\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  if (flags.flip) {\n"
    "    pixels = Pixels(pixels.p1, pixels.p0, pixels.p3, pixels.p2);\n"
    "    pxCoords.x = 1.0 - pxCoords.x;\n"
    "  }\n"
    "\n"
    "  Pattern pattern = pattern(pixels, flags.edgeWeight, flags.triangle, pxCoords);\n"
    "\n"
    "  lowp vec3 final = blend(\n"
    "    blend(pattern.pixels.p0, pattern.pixels.p1, pattern.weights.x),\n"
    "    blend(pattern.pixels.p2, pattern.pixels.p3, pattern.weights.y),\n"
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
