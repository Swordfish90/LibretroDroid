//
// Created by swordfish on 18/11/19.
//

#ifndef LIBRETRODROID_SHADERMANAGER_H
#define LIBRETRODROID_SHADERMANAGER_H

#include <string>

namespace LibretroDroid {

class ShaderManager {
public:
    static const int SHADER_DEFAULT = 0;
    static const int SHADER_CRT = 1;
    static const int SHADER_LCD = 2;

private:
    static const std::string defaultShader;
    static const std::string crtShader;
    static const std::string lcdShader;

public:
    static std::string getShader(int type);

};

}

#endif //LIBRETRODROID_SHADERMANAGER_H
