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

uint Renderer::gen_buffer_texture(std::vector<vec3> &buff) {
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
    // TODO: 拆分为两个update level
    textureHandlesBuffer.clear();
    materialBuffer.clear();
//    meshInfoBuffer.clear();
//    meshIndexMap.clear();

    std::vector<std::pair<Mesh*, mat4>> allMeshes;
    scene->fetch_meshes(scene, mat4(1), allMeshes);

    std::map<Texture*, uint> textureIndexMap;

    triangle_buff.clear();
    lightidx_buff.clear();
    triangle_num = 0;
    light_num = 0;
    for(auto &[u, mat]: allMeshes) {
        assert(u->material);
        auto umtexs = u->material->textures();
        for(Texture *t: umtexs) {
            if(!textureIndexMap.count(t)) {
                textureIndexMap[t] = textureHandlesBuffer.size();
                textureHandlesBuffer.push_back(t->textureHandle);
            }
        }
        uint materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);

        for(auto &t: u->triangles) {
            for(auto & v : t.vertex) triangle_buff.emplace_back(v);
            for(auto & n : t.normal) triangle_buff.emplace_back(n);
            triangle_buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            triangle_buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            triangle_buff.emplace_back(materialPtr, 0, 0);
            triangle_index[&t] = triangle_num; // TODO TMP
//            if(u->material->is_emit) {
//                lightidx_buff.emplace_back(triangle_num, 0, 0);
//                light_num++;
//            }
            triangle_num++;
        }

//        meshInfoBuffer.emplace_back(mat, false, vec3(0.0), materialPtr);
    }

    if(textureHandleSSBO) glDeleteBuffers(1, &textureHandleSSBO);
    glCreateBuffers(1, &textureHandleSSBO);
    glNamedBufferStorage(
            textureHandleSSBO,
            sizeof(GLuint64) * textureHandlesBuffer.size(),
            (const void *)textureHandlesBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );

    if(materialSSBO) glDeleteBuffers(1, &materialSSBO);
    glCreateBuffers(1, &materialSSBO);
    glNamedBufferStorage(
            materialSSBO,
            sizeof(float) * materialBuffer.size(),
            (const void *)materialBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );

    if(triangle_texbuff) glDeleteTextures(1, &triangle_texbuff);
    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
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
    bind_texture("triangles", triangle_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("bvhnodes",  bvhnodes_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("lightidxs", lightidx_texbuff, GL_TEXTURE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureHandleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialSSBO);
    RenderPass::draw();
}

