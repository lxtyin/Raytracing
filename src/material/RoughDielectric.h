//
// Created by 19450 on 2024/3/12.
//

#ifndef PATH_TRACING_ROUGHDIELECTRIC_H
#define PATH_TRACING_ROUGHDIELECTRIC_H

#include "Material.h"


class RoughDielectric: public Material {
    const int materialType = 2;
public:
    vec3 albedo = vec3(0.5);
    float roughness = 0.5;
    Texture *albedo_map = nullptr;
    float indexOfRefraction = 1.3;

    int material_type() override;
    void insert_gui() override;
    std::vector<Texture*> textures() override;
    int insert_buffer(std::vector<float> &materialBuffer, const std::map<Texture*, uint> &textureIndexMap) override;

};


#endif //PATH_TRACING_ROUGHDIELECTRIC_H
