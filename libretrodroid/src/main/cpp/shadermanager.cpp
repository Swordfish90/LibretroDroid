//
// Created by swordfish on 18/11/19.
//

#include "shadermanager.h"

const std::string LibretroDroid::ShaderManager::defaultShader =
        "precision mediump float;\n"
        "uniform lowp sampler2D texture;\n"
        "varying vec2 coords;\n"
        "void main() {\n"
        "  vec4 tex = texture2D(texture, coords);"
        "  gl_FragColor = vec4(tex.rgb, 1.0);\n"
        "}\n";

const std::string LibretroDroid::ShaderManager::crtShader =
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

const std::string LibretroDroid::ShaderManager::lcdShader =
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

const std::string LibretroDroid::ShaderManager::defaultSharp =
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

std::string LibretroDroid::ShaderManager::getShader(Type type) {
    switch (type) {
        case Type::SHADER_DEFAULT:
            return defaultShader;

        case Type::SHADER_CRT:
            return crtShader;

        case Type::SHADER_LCD:
            return lcdShader;

        case Type::SHADER_SHARP:
            return defaultSharp;
    }
}
