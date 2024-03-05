
#include "Texture.h"
#include "glad/glad.h"
#include <iostream>
#include <opencv2/opencv.hpp>

void Texture::load_to_gpu() {
    int format;
    if(channel == 1) format = GL_RED;
    if(channel == 3) format = GL_RGB;
    if(channel == 4) format = GL_RGBA;

    glGenTextures(1, &textureObject);
    glBindTexture(GL_TEXTURE_2D, textureObject);//和VBO同理，绑定后通过GL_TEXTURE_2D赋值
    glTexImage2D(GL_TEXTURE_2D,     //目标
                 0,                 //多级渐进纹理级别
                 format,            //保存的格式
                 width, height,     //长宽
                 0,                 //历史遗留
                 format,            //原图格式
                 GL_UNSIGNED_BYTE,  //存储为Byte数组
                 data.data());             //数据源
    //设置环绕方式，放大缩小时的过滤模式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //第二个参数s,t,r相当于x,y,z，对单一轴设置环绕方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); //此处为简单复制，也是默认方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //设置放大缩小时的插值方式
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST 为邻近
    glGenerateMipmap(GL_TEXTURE_2D);//生成mipmap

    textureHandle = glGetTextureHandleARB(textureObject);
    glMakeTextureHandleResidentARB(textureHandle);
    isonGPU = true;
}


void Texture::unload_from_gpu() {
    glMakeTextureHandleNonResidentARB(textureHandle);
    glDeleteTextures(1, &textureObject);
    textureObject = textureHandle = 0;
    isonGPU = false;
}


Texture::Texture(const string &imagePath) {
    //openGL的图像坐标系是→↑，而一般图像为→↓，加载时默认翻转y轴
    stbi_set_flip_vertically_on_load(true);
    //保存图像内存
    try {
        unsigned char* tmp = stbi_load(imagePath.c_str(), &width, &height, &channel, 0);

        data.resize(width * height * channel);
        memcpy(data.data(), tmp, data.size());

        stbi_image_free(tmp);
    } catch(...) {
        std::cerr << imagePath << ": Texture load failed." << std::endl;
    }
    load_to_gpu();
}

Texture::Texture(int w, int h, int c, std::vector<uchar> &&d):
        width(w), height(h), channel(c){
    data = d;
    load_to_gpu();

}

void Texture::savephoto(const string& path) {
    int format, convert = 0;
    if(channel == 1) format = CV_8U;
    if(channel == 3) format = CV_8UC3, convert = cv::COLOR_RGB2BGR;
    if(channel == 4) format = CV_8UC4, convert = cv::COLOR_RGBA2BGRA;
    cv::Mat img(width, height, format);
    memcpy(img.data, data.data(), width * height * channel);

    cv::Mat fliped, cvted;
    cv::flip(img, fliped, 0);
    if(!convert) {
        cv::imwrite(path, fliped);
    } else {
        cv::cvtColor(fliped, cvted, convert);
        cv::imwrite(path, cvted);
    }
}