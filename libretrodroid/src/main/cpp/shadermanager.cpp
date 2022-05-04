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

const std::string ShaderManager::defaultShader =
        "precision mediump float;\n"
        "uniform lowp sampler2D texture;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "  vec4 tex = texture2D(texture, coords);"
        "  gl_FragColor = vec4(tex.rgb, 1.0);\n"
        "}\n";

const std::string ShaderManager::crtShader =
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
        "   lowp vec3 texel = texture2D(texture, coords).rgb;\n"
        "   lowp vec3 pixelHigh = ((1.0 + BRIGHTBOOST) - (0.2 * texel)) * texel;\n"
        "   lowp vec3 pixelLow  = ((1.0 - INTENSITY) + (0.1 * texel)) * texel;\n"
        "\n"
        "   HIGHP vec2 coords = fract(screenCoords) * 2.0 - vec2(1.0);\n"
        "\n"
        "   lowp float mask = 1.0 - abs(coords.y);\n"
        "\n"
        "   gl_FragColor = vec4(mix(texel, mix(pixelLow, pixelHigh, mask), screenMaskStrength), 1.0);\n"
        "}\n";

const std::string ShaderManager::lcdShader =
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
        "   HIGHP vec2 coords = fract(screenCoords) * 2.0 - vec2(1.0);\n"
        "   coords = coords * coords;\n"
        "\n"
        "   lowp float mask = 1.0 - coords.x - coords.y;\n"
        "\n"
        "   gl_FragColor = vec4(mix(texel, mix(pixelLow, pixelHigh, mask), screenMaskStrength), 1.0);\n"
        "}\n";

const std::string ShaderManager::defaultSharp =
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

const std::string ShaderManager::triangleUpscale =
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
    "varying mediump vec2 coords;\n"
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
    "  mediump vec2 c1 = ((floor(screenCoords) + vec2(0.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c2 = ((floor(screenCoords) + vec2(1.0, 0.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c3 = ((floor(screenCoords) + vec2(1.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "  mediump vec2 c4 = ((floor(screenCoords) + vec2(0.0, 1.0)) + vec2(0.5)) / textureSize;\n"
    "\n"
    "  lowp vec3 t1 = texture2D(texture, c1).rgb;\n"
    "  lowp vec3 t2 = texture2D(texture, c2).rgb;\n"
    "  lowp vec3 t3 = texture2D(texture, c3).rgb;\n"
    "  lowp vec3 t4 = texture2D(texture, c4).rgb;\n"
    "\n"
    "  mediump vec2 localCoords = fract(screenCoords);\n"
    "  lowp vec3 final = nearest(t1, t2, t3, t4, localCoords);\n"
    "\n"
    "  lowp float d1 = 1.0 - step(SAME_COLOR, dist(t1, t3));\n"
    "  lowp float d2 = 1.0 - step(SAME_COLOR, dist(t2, t4));\n"
    "\n"
    "  lowp vec3 p1 = mix(t1, t3, 0.5);\n"
    "  lowp vec3 p2 = mix(t2, t4, 0.5);\n"
    "\n"
    "  lowp float mask = diamond(localCoords);\n"
    "\n"
    "  final = mix(final, p1, d1 * (1.0 - d2) * mask);"
    "  final = mix(final, p2, d2 * (1.0 - d1) * mask);"
    "  final = mix(final, mix(p1, p2, 0.5), step(1.5, d1 + d2) * mask);"
    "\n"
    "  gl_FragColor = vec4(final, 1.0);\n"
    "}\n";

std::string ShaderManager::getShader(Type type) {
    switch (type) {
        case Type::SHADER_DEFAULT:
            return defaultShader;

        case Type::SHADER_CRT:
            return crtShader;

        case Type::SHADER_LCD:
            return lcdShader;

        case Type::SHADER_SHARP:
            return defaultSharp;

        case Type::SHADER_TRIANGLE_UPSCALE:
            return triangleUpscale;
    }
}

} //namespace libretrodroid
