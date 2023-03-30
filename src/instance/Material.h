//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_MATERIAL_H
#define PATH_TRACING_MATERIAL_H

#include "Triangle.h"
#include "../texture/Texture.h"

class Material {
public:
    string name = "A Material";

    vec3 base_color = vec3(1, 0, 1);
    vec3 emission = vec3(100);
    bool is_emit = false;
    float metallic = 0;
    float roughness = 1;
    float specular = 0;
    float specular_tint = 1;
    float sheen = 0;
    float sheen_tint = 0;
    float subsurface = 0;
    float clearcoat = 0;
    float clearcoat_gloss = 0;
    float anisotropic = 0;
    float index_of_refraction = 1;
    float spec_trans = 0;

    Texture *diffuse_map = nullptr;
    Texture *metalness_map = nullptr;
    Texture *roughness_map = nullptr;
    Texture *normal_map = nullptr;

    /**
     * Insert this material into current image gui.
     */
    void insert_gui();
};


#endif //PATH_TRACING_MATERIAL_H
