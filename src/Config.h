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

class Config {
 public:
    static int WINDOW_W;
    static int WINDOW_H;

	static int SPP;
    static int MaxDepth;

	static bool useTAA;
    static bool SVGFTemporalFilter;
    static int SVGFSpatialFilterLevel;
    static bool SVGFForDI, SVGFForIDI;
    static bool useStaticBlender;

    static bool BRDFSampling;
    static bool SkyboxSampling;
    static bool SkyboxLighting;
    static bool DynamicBVH;
    static bool RasterizaionFor1st;

    static void insert_gui();
};

inline int Config::WINDOW_W = 1100;
inline int Config::WINDOW_H = 800;

inline int Config::SPP = 1;
inline int Config::MaxDepth = 1;

inline bool Config::useTAA = false;
inline bool Config::SVGFTemporalFilter = false;
inline int Config::SVGFSpatialFilterLevel = 0;
inline bool Config::SVGFForDI = false;
inline bool Config::SVGFForIDI = false;
inline bool Config::useStaticBlender = false;

inline bool Config::BRDFSampling = false;
inline bool Config::SkyboxSampling = false;
inline bool Config::SkyboxLighting = true;
inline bool Config::DynamicBVH = false;
inline bool Config::RasterizaionFor1st = false;


inline void Config::insert_gui() {
    ImGui::SeparatorText("Config");

    ImGui::SliderInt("Samples per pixel", &Config::SPP, 1, 16);
    ImGui::SliderInt("Max depth", &Config::MaxDepth, 1, 8);

    ImGui::Checkbox("Use TAA", &Config::useTAA); ImGui::SameLine();
    ImGui::Checkbox("Use StaticBlender", &Config::useStaticBlender);

    ImGui::Checkbox("SVGFForDI", &Config::SVGFForDI);  ImGui::SameLine();
    ImGui::Checkbox("SVGFForIDI", &Config::SVGFForIDI);  ImGui::SameLine();
    ImGui::Checkbox("SVGFTemporalFilter", &Config::SVGFTemporalFilter);
    ImGui::SliderInt("SVGFSpatialFilterLevel", &Config::SVGFSpatialFilterLevel, 0, 5);

    ImGui::Checkbox("SkyboxLighting", &Config::SkyboxLighting);  ImGui::SameLine();
    ImGui::Checkbox("SkyboxSampling", &Config::SkyboxSampling);  ImGui::SameLine();
    ImGui::Checkbox("BRDFSampling", &Config::BRDFSampling);

    ImGui::Checkbox("DynamicBVH", &Config::DynamicBVH);  ImGui::SameLine();
    ImGui::Checkbox("RasterizaionFor1st", &Config::RasterizaionFor1st);


}


#endif //PATH_TRACING_CONFIG_H
