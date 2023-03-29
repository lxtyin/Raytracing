//
// Created by lx_tyin on 2023/3/10.
//

#include "HDRTexture.h"
#include "../../include/hdrloader.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include "glm/gtc/type_ptr.hpp"
using glm::vec3;

HDRTexture::HDRTexture(const string &imagePath) {
    HDRLoaderResult hdrRes;
    bool r = HDRLoader::load(imagePath.c_str(), hdrRes);
    if(!r) {
        std::cerr << "HDRTexture: load failed." << std::endl;
        return;
    }
    width = hdrRes.width;
    height = hdrRes.height;


    // image space >(x) v(y)
    vector pdf(height, vector<double>(width));
    vector spdf(height, vector<double>(width));
    float *cache = new float[width * height * 3];

    for(int i = 0;i < height;i++) {
        for(int j = 0;j < width;j++) {
            vec3 col {
                    hdrRes.cols[(i * width + j) * 3 + 0],
                    hdrRes.cols[(i * width + j) * 3 + 1],
                    hdrRes.cols[(i * width + j) * 3 + 2]
            };
            pdf[i][j] = col[0] * 0.2 + col[1] * 0.7 + col[2] * 0.1;
            Light_SUM += pdf[i][j];
        }
    }
    double FY[height];
    for(int i = 0;i < height;i++) {
        for(int j = 0;j < width;j++) {
            if(j != 0) spdf[i][j] = pdf[i][j] + spdf[i][j - 1];
            else spdf[i][j] = pdf[i][j];
        }
        FY[i] = spdf[i][width - 1];
        if(i != 0) FY[i] += FY[i - 1];
    }
    for(int i = 0;i < height;i++) {
        for(int j = 0;j < width;j++) {
            double u = (float)i / height;
            double v = (float)j / width;
            int Y = std::lower_bound(FY, FY + height, u * Light_SUM) - FY;
            int X = std::lower_bound(spdf[Y].begin(), spdf[Y].end(), v * spdf[Y][width - 1]) - spdf[Y].begin();
            cache[(i * width + j) * 3 + 0] = (float)X / width;
            cache[(i * width + j) * 3 + 1] = (float)Y / height;
            cache[(i * width + j) * 3 + 2] = pdf[Y][X] / Light_SUM;
        }
    }

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


    glGenTextures(1, &sample_cache_tto);
    glBindTexture(GL_TEXTURE_2D, sample_cache_tto);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGB32F,
                 width, height,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 cache);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    delete []cache;

}
