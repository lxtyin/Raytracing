//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_RENDERPASS_H
#define PATH_TRACING_RENDERPASS_H

#include "Scene.h"
#include <map>
#include <string>
using std::string;
using uint = unsigned int;

class RenderPass {
protected:
    int tex_n = 0;
    uint VAO = -1, VBO = -1, FBO = -1, FBO_TEX = -1;
public:
    uint shaderProgram = -1;

    explicit RenderPass(const string &frag_shader_path, bool to_screen = false);

    void set_prev_texture(uint id);

    uint draw();
};


#endif //PATH_TRACING_RENDERPASS_H
