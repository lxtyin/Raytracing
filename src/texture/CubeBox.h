//
// Created by lx_tyin on 2023/3/10.
//

#ifndef PATH_TRACING_CUBEBOX_H
#define PATH_TRACING_CUBEBOX_H

#include "Texture.h"

class CubeBox: public Texture {
public:
    CubeBox(const string &px, const string &nx, const string &py,
            const string &ny, const string &pz, const string &nz);

};


#endif //PATH_TRACING_CUBEBOX_H
