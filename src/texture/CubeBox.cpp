//
// Created by lx_tyin on 2023/3/10.
//

#include "CubeBox.h"
#include <iostream>

CubeBox::CubeBox(const string &px, const string &nx, const string &py, const string &ny, const string &pz,
                 const string &nz) {
    int width, height, channel;
    auto load = [&](const string &f, int i) {
        try {
            unsigned char* data = stbi_load(f.c_str(), &width, &height, &channel, 0);
            int format;
            if(channel == 1) format = GL_RED;
            if(channel == 3) format = GL_RGB;
            if(channel == 4) format = GL_RGBA;

            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,     //目标
                         0,                                      //多级渐进纹理级别
                         format,                                 //保存的格式
                         width, height,                          //长宽
                         0,                                      //历史遗留
                         format,                                 //原图格式
                         GL_UNSIGNED_BYTE,                       //存储为Byte数组
                         data);                                  //数据源
            stbi_image_free(data);
        }  catch(const char *msg) {
            std::cerr << "Tool: Fail to load " << f << std::endl;
        }
    };

    glGenTextures(1, &TTO);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TTO);
    load(px, 0), load(nx, 1), load(py, 2);
    load(ny, 3), load(pz, 4), load(nz, 5);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}