//
// Created by lx_tyin on 2023/3/10.
//

#ifndef PATH_TRACING_SIMPLETEXTURE_H
#define PATH_TRACING_SIMPLETEXTURE_H

#include "Texture.h"

class SimpleTexture: public Texture {
    void load();
public:
    int width;
    int height;
    int channel;
    vector<uint> data;

    explicit SimpleTexture(const string &imagePath);

    SimpleTexture(int width, int height, int channel, vector<uint> &data);
};


#endif //PATH_TRACING_SIMPLETEXTURE_H
