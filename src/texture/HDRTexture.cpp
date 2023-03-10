//
// Created by lx_tyin on 2023/3/10.
//

#include "HDRTexture.h"
#include "../../include/hdrloader.h"
#include <iostream>

HDRTexture::HDRTexture(const string &imagePath) {
    HDRLoaderResult hdrRes;
    bool r = HDRLoader::load(imagePath.c_str(), hdrRes);
    if(!r) {
        std::cerr << "HDRTexture: load failed." << std::endl;
        return;
    }
    width = hdrRes.width;
    height = hdrRes.height;

    glGenTextures(1, &TTO);
    glBindTexture(GL_TEXTURE_2D, TTO);//和VBO同理，绑定后通过GL_TEXTURE_2D赋值
    glTexImage2D(GL_TEXTURE_2D,     //目标
                 0,                 //多级渐进纹理级别
                 GL_RGB32F,            //保存的格式
                 width, height,     //长宽
                 0,                 //历史遗留
                 GL_RGB,            //原图格式
                 GL_FLOAT,
                 hdrRes.cols);             //数据源
    //设置环绕方式，放大缩小时的过滤模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //第二个参数s,t,r相当于x,y,z，对单一轴设置环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //此处为简单复制，也是默认方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //设置放大缩小时的插值方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST 为邻近
    glGenerateMipmap(GL_TEXTURE_2D);//生成mipmap


}
