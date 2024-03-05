//
// Created by lx_tyin on 2023/3/10.
//

#ifndef PATH_TRACING_HDRTEXTURE_H
#define PATH_TRACING_HDRTEXTURE_H

#include "Texture.h"


class HDRTexture: public GPUResource {
public:
    int width;
    int height;
    int channel;
    float *data;

    HDRTexture(const string &imagePath);
    ~HDRTexture();
};


#endif //PATH_TRACING_HDRTEXTURE_H
