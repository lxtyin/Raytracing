//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_SKYBOX_H
#define PATH_TRACING_SKYBOX_H

#include "HDRTexture.h"

class Skybox: public HDRTexture {
public:
    double lightSum; // 亮度总和

    GLuint textureObject = 0;
    GLuint64 textureHandle = 0;
    GLuint skyboxsamplerObject; /** < 对HDR贴图重要性采样的cache tto > **/

    virtual void load_to_gpu();
    virtual void unload_from_gpu();
    Skybox(const string &imagePath);
};


#endif //PATH_TRACING_SKYBOX_H
