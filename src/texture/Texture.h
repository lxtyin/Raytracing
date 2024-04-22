#ifndef MAIN_CPP_TEXTURE_H
#define MAIN_CPP_TEXTURE_H

#include "glad/glad.h"
#include "stb_image.h"
#include "../Config.h"
#include <string>
#include <vector>
using std::string;
using uint = unsigned int;
using uchar = unsigned char;


// Store in texture space. (Quadrant 1)
class Texture {
public:
    GLuint textureObject = 0;
    GLuint64 textureHandle = 0;

    int width;
    int height;
    int channel;
    std::vector<uchar> data;

    void load_to_gpu();
    void unload_from_gpu();

    explicit Texture(const string &imagePath);

    Texture(int width, int height, int channel, std::vector<uchar> &&data);

    void savephoto(const string &path);

};

#endif //MAIN_CPP_TEXTURE_H
