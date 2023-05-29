//
// Created by 19450 on 2023/5/22.
//

#ifndef PATH_TRACING_SRC_TOOL_EXGLFW_H_
#define PATH_TRACING_SRC_TOOL_EXGLFW_H_

#include "glfw/glfw3.h"

bool glfwGetKeyDown(GLFWwindow* window, int key) {
	static int key_state[130] = {0};
	bool cur = glfwGetKey(window, key);
	if(!key_state[key] && cur) {
		key_state[key] = cur;
		return true;
	}
	key_state[key] = cur;
	return false;
}

#endif //PATH_TRACING_SRC_TOOL_EXGLFW_H_
