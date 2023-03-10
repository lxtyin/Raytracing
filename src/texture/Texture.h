#ifndef MAIN_CPP_TEXTURE_H
#define MAIN_CPP_TEXTURE_H

#include "glad/glad.h"
#include "stb_image.h"
#include <string>
#include <vector>
using std::string;
using std::vector;
using uint = unsigned int;

//纹理对象，一张图片
class Texture{
public:
    uint TTO;

	/**
	 * 设置一些参数
	 * \param pname
	 * \param params
	 */
    void setParameter(int pname, int params);
};

#endif //MAIN_CPP_TEXTURE_H
