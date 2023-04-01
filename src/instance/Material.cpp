//
// Created by lx_tyin on 2023/2/20.
//

#include "Material.h"
#include "imgui/imgui.h"

void Material::insert_gui() {
    if(ImGui::TreeNode(name.c_str())) {

        float col1[3] = { base_color.x, base_color.y, base_color.z };
        ImGui::ColorEdit3("Base Color", col1);
        base_color = {col1[0], col1[1], col1[2]};

        ImGui::SliderFloat("Roughness", &roughness, 0, 1);
        ImGui::SliderFloat("Metallic", &metallic, 0, 1);
		ImGui::SliderFloat("Specular", &specular, 0, 1);
		ImGui::SliderFloat("SpecularTint", &specular_tint, 0, 1);
        ImGui::SliderFloat("IndexOfRefraction", &index_of_refraction, 1, 3);
        ImGui::SliderFloat("SpecTrans", &spec_trans, 0, 1);
        
        if(is_emit) {
            for(int i = 0;i < 3;i++) col1[i] = emission[i];
            ImGui::ColorEdit3("Emission", col1);
            emission = {col1[0], col1[1], col1[2]};
        }
        if(diffuse_map) {
            ImGui::Text("Diffuse map Exists");
        }

        ImGui::TreePop();
    }

}
