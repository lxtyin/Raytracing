//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_ROUGHCONDUCTOR_H
#define PATH_TRACING_ROUGHCONDUCTOR_H

#include "Material.h"

/**
 * Simplified roughConductor, where albedo = F0
 */
class RoughConductor: public Material {
public:
    int materialType = 1;

    vec3 albedo = vec3(0.5);
    float roughness = 0.5;
    Texture *albedo_map = nullptr;

    int material_type() override;
    void insert_gui() override;
    std::vector<Texture*> textures() override;
    int insert_buffer(std::vector<float> &materialBuffer, const std::map<Texture*, uint> &textureIndexMap) override;
};




#endif //PATH_TRACING_ROUGHCONDUCTOR_H
