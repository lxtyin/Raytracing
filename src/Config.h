//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_CONFIG_H
#define PATH_TRACING_CONFIG_H

#include "imgui/imgui.h"
#include "common.h"

// RenderDoc does not support them, so we need to disable texture.
#define USE_BINDLESS_TEXTURE

const int SCREEN_W = 640;
const int SCREEN_H = 480;

// FOV for X axis.
const float FOV_X = M_PI / 3;

enum VisualizeType {
    Visual_RENDER,
    Visual_DIRECT,
    Visual_INDIRECT,
    Visual_ALBEDO,
    Visual_NORMAL,
    Visual_DEPTH
};

class Config {
 public:
    static int WINDOW_W;
    static int WINDOW_H;

	static int SPP;
    static int MaxDepth;

	static bool useTAA;
    static bool SVGF;
    static int SVGFSpatialFilterLevel;
    static bool SVGFForDI, SVGFForIDI;
    static bool useStaticBlender;

    static bool BRDFSampling;
    static bool SkyboxSampling;
    static bool SkyboxLighting;
    static bool DynamicBVH;
    static bool RasterizaionFor1st;

    static VisualizeType visualType;

    static void insert_gui();
};

inline int Config::WINDOW_W = 1100;
inline int Config::WINDOW_H = 800;

inline int Config::SPP = 1;
inline int Config::MaxDepth = 2;

inline bool Config::useTAA = false;
inline bool Config::SVGF = false;
inline int Config::SVGFSpatialFilterLevel = 0;
inline bool Config::SVGFForDI = false;
inline bool Config::SVGFForIDI = false;
inline bool Config::useStaticBlender = false;

inline bool Config::BRDFSampling = true;
inline bool Config::SkyboxSampling = true;
inline bool Config::SkyboxLighting = true;
inline bool Config::DynamicBVH = true;
inline bool Config::RasterizaionFor1st = true;

inline VisualizeType Config::visualType = Visual_RENDER;

inline void Config::insert_gui() {
    ImGui::SeparatorText("Config");

    ImGui::SliderInt("Samples per pixel", &Config::SPP, 1, 16);
    ImGui::SliderInt("Max depth", &Config::MaxDepth, 1, 8);

    ImGui::Checkbox("Use TAA", &Config::useTAA); ImGui::SameLine();
    ImGui::Checkbox("Use StaticBlender", &Config::useStaticBlender);

    ImGui::Checkbox("SVGFForDI", &Config::SVGFForDI);  ImGui::SameLine();
    ImGui::Checkbox("SVGFForIDI", &Config::SVGFForIDI);  ImGui::SameLine();
    ImGui::Checkbox("SVGF", &Config::SVGF);
    ImGui::SliderInt("SVGFSpatialFilterLevel", &Config::SVGFSpatialFilterLevel, 0, 5);

    ImGui::Checkbox("SkyboxLighting", &Config::SkyboxLighting);  ImGui::SameLine();
    ImGui::Checkbox("SkyboxSampling", &Config::SkyboxSampling);  ImGui::SameLine();
    ImGui::Checkbox("BRDFSampling", &Config::BRDFSampling);

    ImGui::Checkbox("DynamicBVH", &Config::DynamicBVH);  ImGui::SameLine();
    ImGui::Checkbox("RasterizaionFor1st", &Config::RasterizaionFor1st);

    static ImGuiComboFlags flags = ImGuiComboFlags_HeightSmall | ImGuiComboFlags_PopupAlignLeft;
    const char* items[] = { "Render", "Direct", "Indirect", "Albedo", "Normal", "Depth"};
    int item_current_idx = Config::visualType;
    const char* combo_preview_value = items[item_current_idx];
    if (ImGui::BeginCombo("Visualize", combo_preview_value, flags)) {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++) {
            const bool is_selected = (item_current_idx == n);

            if (ImGui::Selectable(items[n], is_selected)) {
                if(n == item_current_idx) continue;
                else {
                    Config::visualType = static_cast<VisualizeType>(n);
                    item_current_idx = n;
                }
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

}


#endif //PATH_TRACING_CONFIG_H
