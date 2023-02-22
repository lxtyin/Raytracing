//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_MATERIAL_H
#define PATH_TRACING_MATERIAL_H

#include "Triangle.h"

class Material {
public:
    vec3 color = vec3(1);
    bool is_emit = false;
    vec3 emission;
};


#endif //PATH_TRACING_MATERIAL_H
