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

// FOV for X axis.
const float SCREEN_FOV = M_PI / 3;

class Config {
 public:
	static int SPP;
	static bool useTAA;
	static int filterLevel;
};

inline int Config::SPP = 1;
inline bool Config::useTAA = false;
inline int Config::filterLevel = 0;


#endif //PATH_TRACING_CONFIG_H
