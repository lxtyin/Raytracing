//
// Created by lx_tyin on 2023/3/10.
//

#ifndef PATH_TRACING_HDRTEXTURE_H
#define PATH_TRACING_HDRTEXTURE_H

#include "Texture.h"


class HDRTexture: public Texture {
public:
    int width;
    int height;
    int channel;

    uint sample_cache_tto; /** < 对HDR贴图重要性采样的cache tto > **/

    HDRTexture(const string &imagePath);
};


#endif //PATH_TRACING_HDRTEXTURE_H
