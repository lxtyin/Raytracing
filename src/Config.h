//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_CONFIG_H
#define PATH_TRACING_CONFIG_H

#include "imgui/imgui.h"
#include "common.h"

const int SCREEN_W = 1080;
const int SCREEN_H = 720;
#define SCREEN_FOV (M_PI / 3)

class Config {
 public:
	static int SPP;
	static bool is_filter_enabled;
	static bool is_motionvector_enabled;

	static void insert_gui() {
		if(ImGui::TreeNode("Config")) {
			ImGui::Checkbox("Use Filter", &is_filter_enabled);
			ImGui::Checkbox("Use Motion Vector", &is_motionvector_enabled);
			ImGui::SliderInt("Samples per pixel", &SPP, 1, 32);
			ImGui::TreePop();
		}
	}
};

inline int Config::SPP = 1;
inline bool Config::is_filter_enabled = false;
inline bool Config::is_motionvector_enabled = false;


#endif //PATH_TRACING_CONFIG_H
