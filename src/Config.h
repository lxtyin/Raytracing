//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_CONFIG_H
#define PATH_TRACING_CONFIG_H

#include "imgui/imgui.h"
#include "common.h"

const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int WINDOW_W = 1000;
const int WINDOW_H = 700;
#define SCREEN_FOV (M_PI / 3)

class Config {
 public:
	static int SPP;
	static bool is_filter_enabled;
	static bool is_motionvector_enabled;

	static void insert_gui() {
        ImGui::SeparatorText("Config");

        ImGui::Checkbox("Use Filter", &is_filter_enabled);
        ImGui::Checkbox("Use Motion Vector", &is_motionvector_enabled);
        ImGui::SliderInt("Samples per pixel", &SPP, 1, 32);
	}
};

inline int Config::SPP = 1;
inline bool Config::is_filter_enabled = false;
inline bool Config::is_motionvector_enabled = false;


#endif //PATH_TRACING_CONFIG_H
