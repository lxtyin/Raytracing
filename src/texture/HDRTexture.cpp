//
// Created by lx_tyin on 2023/3/10.
//

#include "HDRTexture.h"
#include "../../include/hdrloader.h"
#include <iostream>
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
    channel = 3;
    data = hdrRes.cols;
}


HDRTexture::~HDRTexture() {
    delete data;
}

