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
    vec3 emission = vec3(100);
    float specular = 0;     // 镜面光强度
    float roughness = 1;    // 粗糙度 [0, 1]
    float metallic = 0;     // 金属度 缩减漫反射 [0, 1]
    float subsurface = 0;   // 次表面散射 [0, 1]
};


#endif //PATH_TRACING_MATERIAL_H
