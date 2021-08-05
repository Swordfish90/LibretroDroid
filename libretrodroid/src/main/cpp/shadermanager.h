//
// Created by swordfish on 18/11/19.
//

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
        SHADER_SHARP = 3
    };

private:
    static const std::string defaultShader;
    static const std::string crtShader;
    static const std::string lcdShader;
    static const std::string defaultSharp;

public:
    static std::string getShader(Type type);

};

}

#endif //LIBRETRODROID_SHADERMANAGER_H
