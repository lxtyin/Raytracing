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
    const int materialType = 1;
public:
    vec3 albedo = vec3(0.5);
    float alpha = 0.5;
    Texture *albedo_map = nullptr;

    void insert_gui() override;
    std::vector<Texture*> textures() override;
    uint insert_buffer(std::vector<float> &materialBuffer, const std::map<Texture*, uint> &textureIndexMap) override;
};




#endif //PATH_TRACING_ROUGHCONDUCTOR_H
