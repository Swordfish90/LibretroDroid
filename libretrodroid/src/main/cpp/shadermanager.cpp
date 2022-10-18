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
    "varying mediump vec2 inverseTextureSize;\n"
    "\n"
    "void main() {\n"
    "  mediump vec2 coords = vec2(vCoordinate.x, mix(vCoordinate.y, 1.0 - vCoordinate.y, vFlipY)) * 1.000001;\n"
    "  screenCoords = coords * textureSize - vec2(0.5);\n"
    "  inverseTextureSize = vec2(1.0) / textureSize;\n"
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
    "varying mediump vec2 inverseTextureSize;\n"
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
    "lowp float sharpSmooth(lowp float minv, lowp float maxv, lowp float t, lowp float sharpness) {\n"
    "  lowp float dist = maxv - minv;\n"
    "  return mix(minv, maxv, linearStep(minv + dist * sharpness, maxv - sharpness * dist, t));\n"
    "}\n"
    "lowp vec3 interpolate(lowp vec3 a, lowp vec3 b, lowp vec3 c, lowp vec3 d, lowp float tana, lowp vec2 p, lowp float sharpness) {\n"
    "  lowp float xu = p.x + tana * p.y;\n"
    "  lowp float xd = p.x - tana * (1.0 - p.y);\n"
    "  lowp float maxtana = max(tana, 0.0);\n"
    "  lowp float mintana = min(tana, 0.0);\n"
    "  lowp vec4 tu = vec4(mintana, maxtana, 1.0 + mintana, 1.0 + maxtana);\n"
    "  lowp vec4 td = vec4(-maxtana, -mintana, 1.0 - maxtana, 1.0 - mintana);\n"
    "  lowp vec2 xuw = tu.yz;\n"
    "  lowp vec2 xdw = td.yz;\n"
    "  if (p.x > mix(tu.z, td.z, p.y)) {\n"
    "    xuw = tu.zw;\n"
    "    xdw = td.zw;\n"
    "  } else if (p.x < mix(tu.y, td.y, p.y)) {\n"
    "    xuw = tu.xy;\n"
    "    xdw = td.xy;\n"
    "  }\n"
    "  lowp float wu = sharpSmooth(xuw.x, xuw.y, xu, sharpness);\n"
    "  lowp float wd = sharpSmooth(xdw.x, xdw.y, xd, sharpness);\n"
    "  return mix(mix(a, b, wu), mix(c, d, wd), sharpSmooth(0.0, 1.0, p.y, sharpness));\n"
    "}\n"
    "\n"
    "void main() {\n"
    "  HIGHP vec2 relativeCoords = inverseTextureSize * (floor(screenCoords) + vec2(0.5));\n"
    "  mediump vec2 c1 = relativeCoords;\n"
    "  mediump vec2 c2 = relativeCoords + vec2(inverseTextureSize.x, 0.0);\n"
    "  mediump vec2 c3 = relativeCoords + vec2(inverseTextureSize.x, inverseTextureSize.y);\n"
    "  mediump vec2 c4 = relativeCoords + vec2(0.0, inverseTextureSize.y);\n"
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
    "  lowp float dx = l1 - l2 + l4 - l3;\n"
    "  lowp float dy = l1 + l2 - l4 - l3;\n"
    "\n"
    "  lowp vec2 pxCoords = fract(screenCoords);\n"
    "\n"
    "  bool invertAngle = abs(dy) > abs(dx);\n"
    "  lowp vec3 final = interpolate(\n"
    "    t1,\n"
    "    invertAngle ? t4 : t2,\n"
    "    invertAngle ? t2 : t4,\n"
    "    t3,\n"
    "    invertAngle ? dx / dy : dy / dx,\n"
    "    invertAngle ? pxCoords.yx : pxCoords,\n"
    "    sharpness\n"
    "  );\n"
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

    case Type::SHADER_UPSCALE_CUT_SHARP:
        return {
            cutUpscaleVertex,
            "#define USE_DYNAMIC_SHARPNESS 1\n"
            "#define USE_SHARPENING_BIAS 1\n"
            "#define DYNAMIC_SHARPNESS_MIN 0.10\n"
            "#define DYNAMIC_SHARPNESS_MAX 0.30\n"
            "#define STATIC_SHARPNESS 0.2\n"
            "#define USE_FAST_LUMA 1\n"
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
            "#define STATIC_SHARPNESS 0.2\n"
            "#define USE_FAST_LUMA 1\n"
            + cutUpscaleFragment,
            false
        };
    }
}

} //namespace libretrodroid
