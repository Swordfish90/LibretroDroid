/*
 *     Copyright (C) 2022  Filippo Scognamiglio
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

#ifndef LIBRETRODROID_SHADERMANAGER_H
#define LIBRETRODROID_SHADERMANAGER_H

#include <string>

namespace libretrodroid {

class ShaderManager {
public:
    enum class Type {
        SHADER_DEFAULT = 0,
        SHADER_CRT = 1,
        SHADER_LCD = 2,
        SHADER_SHARP = 3,
        SHADER_TRIANGLE_UPSCALE = 4,
        SHADER_TRIANGLE_UPSCALE_SMOOTH = 5
    };

private:
    static const std::string defaultShaderVertex;

    static const std::string defaultShaderFragment;
    static const std::string defaultSharpFragment;
    static const std::string crtShaderFragment;
    static const std::string lcdShaderFragment;

    static const std::string triangleUpscaleVertex;
    static const std::string triangleUpscaleFragment;
    static const std::string triangleUpscaleSmoothFragment;

public:
    static std::tuple<std::string, std::string> getShader(Type type);

};

}

#endif //LIBRETRODROID_SHADERMANAGER_H
