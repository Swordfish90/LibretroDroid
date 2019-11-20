//
// Created by swordfish on 18/11/19.
//

#ifndef LIBRETRODROID_SHADERMANAGER_H
#define LIBRETRODROID_SHADERMANAGER_H

#include <string>

namespace LibretroDroid {

class ShaderManager {
public:
    static enum class Type {
        SHADER_DEFAULT = 0,
        SHADER_CRT = 1,
        SHADER_LCD = 2
    };

private:
    static const std::string defaultShader;
    static const std::string crtShader;
    static const std::string lcdShader;

public:
    static std::string getShader(Type type);

};

}

#endif //LIBRETRODROID_SHADERMANAGER_H
