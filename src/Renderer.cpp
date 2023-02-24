//
// Created by lx_tyin on 2023/2/20.
//

#include "Renderer.h"
#include "Tool.h"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <queue>

void Renderer::set_buff_toshader(vector<vec3> &buff, const char *name) {

    if(shaderProgram == -1) {
        std::cerr << "Renderer: Has not init yet." << std::endl;
        return;
    }
    glUseProgram(shaderProgram);

    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(GL_TEXTURE_BUFFER, buff.size() * 3 * sizeof(float), buff.data(), GL_STATIC_DRAW);

    glActiveTexture(GL_TEXTURE0 + tex_n);
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_BUFFER, tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);

    glUniform1i(glGetUniformLocation(shaderProgram, name), tex_n++);
}

void Renderer::reload_material(Scene *scene) {
    // <color, emission, (is_emit, . 0), (. . .)> 4x
    auto &buff = material_buff;
    buff.clear();
    for(auto *obj: scene->objects) {
        auto *m = obj->material;
        buff.emplace_back(m->color);
        buff.emplace_back(m->emission);
        buff.emplace_back(m->is_emit, m->specular, 0);
        buff.emplace_back(m->roughness, m->metallic, m->subsurface);

    }
    set_buff_toshader(buff, "materials");
}

void Renderer::reload_triangles(Scene *scene) {
    // <v0, v1, v2, (u0, u1, u2), (v0, v1, v2), (m_index, 0, 0)> 6x
    auto &buff = triangle_buff;
    buff.clear();
    lightindex_buff.clear();
    triangle_index.clear();
    int n = 0, m = 0;
    for(auto *o: scene->objects) {
        for(auto &tr: o->triangles) {
            auto &t = scene->triangles[n];
            if(o->material->is_emit) lightindex_buff.emplace_back(n, 0, 0);
            triangle_index[&t] = n++;
            for(auto & i : t.vertex) buff.push_back(i);
            buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            buff.emplace_back(m, 0, 0);
        }
        m++;
    }
    set_buff_toshader(buff, "triangles");
    set_buff_toshader(lightindex_buff, "lightindexs");

    assert(lightindex_buff.size() > 0); // 场景至少需要一个光源

    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), lightindex_buff.size());
    glUniform1i(glGetUniformLocation(shaderProgram, "triangles_num"), scene->triangles.size());
}

void Renderer::reload_bvhnodes(Scene *scene) {
    // <aa, bb, (l_index, r_index, t_index)> 3x
    auto &buff = bvhnodes_buff;
    buff.resize(scene->bvh_root->siz * 3);

    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->bvh_root, 0);
    int n = 0;
    while(!q.empty()) {
        auto &[p, index] = q.front();
        q.pop();
        int il = -1, ir = -1, it = -1;
        if(p->ls) il = ++n, q.emplace(p->ls, il);
        if(p->rs) ir = ++n, q.emplace(p->rs, ir);
        if(p->isleaf) it = triangle_index[p->triangle];
        buff[index * 3] = p->aa;
        buff[index * 3 + 1] = p->bb;
        buff[index * 3 + 2] = {il, ir, it};
    }
    set_buff_toshader(buff, "bvhnodes");
}

void Renderer::reload_scene(Scene *scene) {
    reload_material(scene);
    reload_triangles(scene);
    reload_bvhnodes(scene);
}

