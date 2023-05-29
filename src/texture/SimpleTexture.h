//
// Created by lx_tyin on 2023/3/10.
//

#ifndef PATH_TRACING_SIMPLETEXTURE_H
#define PATH_TRACING_SIMPLETEXTURE_H

#include "Texture.h"

// 简单图片
class SimpleTexture: public Texture {
    void load();
public:
    int width;
    int height;
    int channel;
    vector<uchar> data;

    explicit SimpleTexture(const string &imagePath);

    SimpleTexture(int width, int height, int channel, vector<uchar> &data);

	void savephoto(const string &path);
};


#endif //PATH_TRACING_SIMPLETEXTURE_H
