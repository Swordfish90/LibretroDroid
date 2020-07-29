//
// Created by swordfish on 18/11/19.
//

#ifndef LIBRETRODROID_SHADERMANAGER_H
#define LIBRETRODROID_SHADERMANAGER_H

#include <string>

namespace LibretroDroid {

struct Shader {
    std::string fragmentShader;
    bool bilinearFiltering;
};

class ShaderManager {
public:
    enum class Type {
        SHADER_DEFAULT = 0,
        SHADER_CRT = 1,
        SHADER_LCD = 2,
        SHADER_SHARP = 3,
    };

private:
    static const std::string defaultShader;
    static const std::string crtShader;
    static const std::string lcdShader;

public:
    static Shader getShader(Type type);

};

}

#endif //LIBRETRODROID_SHADERMANAGER_H
