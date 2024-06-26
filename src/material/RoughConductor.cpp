//
// Created by 19450 on 2024/3/4.
//

#include "RoughConductor.h"
#include "imgui/imgui.h"
#include "../tool/tool.h"
#include <iostream>
#include <map>

void RoughConductor::insert_gui() {
    float col1[3] = { albedo.x, albedo.y, albedo.z };
    ImGui::ColorEdit3("albedo", col1);
    albedo = {col1[0], col1[1], col1[2]};

    ImGui::SliderFloat("roughness", &roughness, 0.001, 1, "%.3f");
    ImGui::SliderFloat("metallic", &metallic, 0, 1, "%.3f");

    if(albedo_map) {
        ImGui::Text("Albedo map");
        ImGui::SameLine();
        if(ImGui::Button("Save albedo map")) albedo_map->savephoto(name + "_albedo_map.jpg");
    }
}

int RoughConductor::insert_buffer(std::vector<float> &materialBuffer, const std::map<Texture*, uint> &textureIndexMap) {
    int ptr = materialBuffer.size();
    materialBuffer.emplace_back((float)material_type());
    materialBuffer.emplace_back(albedo.x);
    materialBuffer.emplace_back(albedo.y);
    materialBuffer.emplace_back(albedo.z);
    materialBuffer.emplace_back(roughness);
    materialBuffer.emplace_back(metallic);
    if(albedo_map && albedo_map->textureHandle) {
        materialBuffer.emplace_back((float)textureIndexMap.at(albedo_map));
    } else materialBuffer.emplace_back(-1.0f);
    return ptr;
}

std::vector<Texture *> RoughConductor::textures() {
    std::vector<Texture*> result;
    if(albedo_map) {
        result.push_back(albedo_map);
    }
    return result;
}

int RoughConductor::material_type() {
    return 1;
}


