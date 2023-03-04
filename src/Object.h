//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_OBJECT_H
#define PATH_TRACING_OBJECT_H

#include <vector>
#include "Triangle.h"
#include "Material.h"
using std::vector;

class Object {
public:
    vector<Triangle> triangles;
    Material *material = nullptr;
    vec3 position = vec3(0);
    vec3 scale = vec3(1);
    vec3 rotation = vec3(0); // xyz

    mat4 transform();
    vec3 direction_x();
    vec3 direction_y();
    vec3 direction_z();
};


#endif //PATH_TRACING_OBJECT_H
