//
// Created by 19450 on 2024/3/13.
//

#ifndef PATH_TRACING_TINYUI_H
#define PATH_TRACING_TINYUI_H

#include "instance/Scene.h"
#include "Config.h"
#include "imgui/imgui.h"
#include "imgui/backend/imgui_impl_glfw.h"
#include "imgui/backend/imgui_impl_opengl3.h"
#include <set>

class TinyUI {
private:
    static bool showUI;

    static Instance *selectedInstance;

    static std::set<Instance*> toopenList;

    static void insert_instance_Hierarchy(Instance *u);

    static void insert_instance_Editor(Instance *u);

    static void insert_configuration();

public:
    /**
     * initialization using glfw.
     */
    static void init(GLFWwindow *window);

    static void update(Scene *scene, float fps);

    static void terminate();

    /**
     * select an instance and show it in hierarchy.
     */
    static void selectInstance(Instance* instance);
};


#endif //PATH_TRACING_TINYUI_H
