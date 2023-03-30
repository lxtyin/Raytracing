//
// Created by lx_tyin on 2023/2/20.
//

#include "Renderer.h"
#include "tool/tool.h"
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

void Renderer::reload_meshes(Scene *scene) {
    triangle_buff.clear();
    lightidx_buff.clear();
    material_buff.clear();
    texture_list.clear();
    material_num = 0;
    triangle_num = 0;
    light_num = 0;
    for(auto &mesh: scene->world_meshes) {
        auto *m = mesh.material;

        int diffuse_map_idx = -1, metalness_map_idx = -1, normal_map_idx = -1, roughness_map_idx = -1;
        if(m->diffuse_map) {
            diffuse_map_idx = (int)texture_list.size();
            texture_list.push_back(m->diffuse_map);
        }
        if(m->metalness_map) {
            metalness_map_idx = (int)texture_list.size();
            texture_list.push_back(m->metalness_map);
        }
        if(m->normal_map) {
            normal_map_idx = (int)texture_list.size();
            texture_list.push_back(m->normal_map);
        }
        if(m->roughness_map) {
            roughness_map_idx = (int)texture_list.size();
            texture_list.push_back(m->roughness_map);
        }

        material_buff.emplace_back(m->base_color);
        material_buff.emplace_back(m->emission);
        material_buff.emplace_back(m->is_emit, m->metallic, m->roughness);
        material_buff.emplace_back(m->specular, m->specular_tint, m->sheen);
        material_buff.emplace_back(m->sheen_tint, m->subsurface, m->clearcoat);
        material_buff.emplace_back(m->clearcoat_gloss, m->anisotropic, m->index_of_refraction);
        material_buff.emplace_back(m->spec_trans, 0, diffuse_map_idx);
        material_buff.emplace_back(metalness_map_idx, roughness_map_idx, normal_map_idx);
        for(auto &t: mesh.triangles) {
            for(auto & v : t.vertex) triangle_buff.emplace_back(v);
            for(auto & n : t.normal) triangle_buff.emplace_back(n);
            triangle_buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            triangle_buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            triangle_buff.emplace_back(material_num, 0, 0);
            triangle_index[&t] = triangle_num;
            if(m->is_emit) {
                lightidx_buff.emplace_back(triangle_num, 0, 0);
                light_num++;
            }
            triangle_num++;
        }
        material_num++;
    }
    if(material_texbuff) glDeleteTextures(1, &material_texbuff);
    if(triangle_texbuff) glDeleteTextures(1, &triangle_texbuff);
    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
    material_texbuff = gen_buffer_texture(material_buff);
    triangle_texbuff = gen_buffer_texture(triangle_buff);
    lightidx_texbuff = gen_buffer_texture(lightidx_buff);
}

void Renderer::reload_bvhnodes(Scene *scene) {
    bvhnodes_buff.resize(scene->bvh_root->siz * B_SIZ);
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
        bvhnodes_buff[index * 3] = p->aa;
        bvhnodes_buff[index * 3 + 1] = p->bb;
        bvhnodes_buff[index * 3 + 2] = {il, ir, it};
    }
    if(bvhnodes_texbuff) glDeleteTextures(1, &bvhnodes_texbuff);
    bvhnodes_texbuff = gen_buffer_texture(bvhnodes_buff);
}

void Renderer::reload_scene(Scene *scene) {
    reload_meshes(scene);
    reload_bvhnodes(scene);
}

void Renderer::draw() {
    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), light_num);
    glUniform1i(glGetUniformLocation(shaderProgram, "triangle_num"), triangle_num);
    bind_texture("materials", material_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("triangles", triangle_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("bvhnodes",  bvhnodes_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("lightidxs", lightidx_texbuff, GL_TEXTURE_BUFFER);
    for(int i = 0;i < texture_list.size();i++) {
        bind_texture(str_format("texture_list[%d]", i).c_str(), texture_list[i]->TTO);
    }
    RenderPass::draw();
}

