#ifndef SHADER_FACTORY_H
#define SHADER_FACTORY_H

#include <string>

std::string getVertexShader();
std::string getFragmentShader();
std::string getVertexShaderString(int textureNum);

std::string getYuv420P2RgbFragmentShader(bool fullRange);
std::string getYuv420Sp2RgbFragmentShader(bool fullRange);
std::string getYuv420P10Le2RgbFragmentShader(bool fullRange);

#endif // SHADER_FACTORY_H
