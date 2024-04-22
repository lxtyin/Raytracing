//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_CONFIG_H
#define PATH_TRACING_CONFIG_H

#include "imgui/imgui.h"
#include "common.h"

// RenderDoc does not support them, so we need to disable texture.
//#define USE_BINDLESS_TEXTURE

const int SCREEN_W = 640;
const int SCREEN_H = 480;

// FOV for X axis.
const float FOV_X = M_PI / 3;

class Config {
 public:
    static int WINDOW_W;
    static int WINDOW_H;

	static int SPP;
	static bool useTAA;
    static bool useTemporalFilter;
    static bool useStaticBlender;
	static int filterLevel;
};

inline int Config::WINDOW_W = 1000;
inline int Config::WINDOW_H = 700;

inline int Config::SPP = 1;
inline bool Config::useTAA = false;
inline bool Config::useTemporalFilter = false;
inline bool Config::useStaticBlender = false;
inline int Config::filterLevel = 0;


#endif //PATH_TRACING_CONFIG_H
