#include "Texture.h"

void Texture::setParameter(int pname, int params) {
    glBindTexture(GL_TEXTURE_2D, TTO);
    glTexParameteri(GL_TEXTURE_2D, pname, params);
}