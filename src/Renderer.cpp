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

uint Renderer::gen_buffer_texture(vector<vec3> &buff) {
    uint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(GL_TEXTURE_BUFFER, buff.size() * 3 * sizeof(float), buff.data(), GL_STATIC_DRAW);

    uint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_BUFFER, tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);
    return tex;
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
    if(material_texbuff) glDeleteTextures(1, &material_texbuff);
    material_texbuff = gen_buffer_texture(buff);
}

void Renderer::reload_triangles(Scene *scene) {
    // <v0, v1, v2, (u0, u1, u2), (v0, v1, v2), (m_index, 0, 0)> 6x
    auto &buff = triangle_buff;
    buff.clear();
    lightidx_buff.clear();
    triangle_index.clear();
    int n = 0, m = 0;
    for(auto *o: scene->objects) {
        for(auto &tr: o->triangles) {
            auto &t = scene->triangles[n];
            if(o->material->is_emit) lightidx_buff.emplace_back(n, 0, 0);
            triangle_index[&t] = n++;
            for(auto & i : t.vertex) buff.push_back(i);
            buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            buff.emplace_back(m, 0, 0);
        }
        m++;
    }
    if(triangle_texbuff) glDeleteTextures(1, &triangle_texbuff);
    triangle_texbuff = gen_buffer_texture(triangle_buff);
    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
    lightidx_texbuff = gen_buffer_texture(lightidx_buff);

    assert(!lightidx_buff.empty()); // 场景至少需要一个光源
    light_t_num = (int)lightidx_buff.size();
    triangle_num = (int)scene->triangles.size();
}

void Renderer::reload_bvhnodes(Scene *scene) {
    // <aa, bb, (l_index, r_index, t_index)> 3x
    auto &buff = bvhnodes_buff;
    buff.resize(scene->bvh_root->siz * 3);

    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->bvh_root, 0);
    int n = 0;
    while(!q.empty()) {
        auto [p, index] = q.front();
        q.pop();
        int il = -1, ir = -1, it = -1;
        if(p->ls) il = ++n, q.emplace(p->ls, il);
        if(p->rs) ir = ++n, q.emplace(p->rs, ir);
        if(p->isleaf) it = triangle_index[p->triangle];
        buff[index * 3] = p->aa;
        buff[index * 3 + 1] = p->bb;
        buff[index * 3 + 2] = {il, ir, it};
    }
    if(bvhnodes_texbuff) glDeleteTextures(1, &bvhnodes_texbuff);
    bvhnodes_texbuff = gen_buffer_texture(bvhnodes_buff);
}

void Renderer::reload_scene(Scene *scene) {
    reload_material(scene);
    reload_triangles(scene);
    reload_bvhnodes(scene);
}

uint Renderer::draw() {
    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), light_t_num);
    glUniform1i(glGetUniformLocation(shaderProgram, "triangle_num"), triangle_num);
    bind_texture("materials", material_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("triangles", triangle_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("bvhnodes",  bvhnodes_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("lightidxs", lightidx_texbuff, GL_TEXTURE_BUFFER);
    return RenderPass::draw();
}

