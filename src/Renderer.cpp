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
    meshInfoBuffer.clear();
    meshIndexMap.clear();
    triangleIndexMap.clear();

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
        int materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);

        for(auto &t: u->triangles) {
            for(auto & v : t.vertex) triangle_buff.emplace_back(v);
            for(auto & n : t.normal) triangle_buff.emplace_back(n);
            triangle_buff.emplace_back(t.uv[0][0], t.uv[1][0], t.uv[2][0]);
            triangle_buff.emplace_back(t.uv[0][1], t.uv[1][1], t.uv[2][1]);
            triangle_buff.emplace_back(0, 0, 0); // placeholder
//            if(u->material->is_emit) {
//                lightidx_buff.emplace_back(triangle_num, 0, 0);
//                light_num++;
//            }
            triangleIndexMap[&t] = triangle_num; // TODO
            triangle_num++;
        }

        assert(!meshIndexMap.count(u));
        meshIndexMap[u] = meshInfoBuffer.size();
        MeshInfo y;
        y.world2local = glm::inverse(mat);
        y.emission = vec4(0);// TODO Emission
        y.materialPtr = materialPtr;
        meshInfoBuffer.push_back(y);
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

    if(meshInfoSSBO) glDeleteBuffers(1, &meshInfoSSBO);
    glCreateBuffers(1, &meshInfoSSBO);
    glNamedBufferStorage(
            meshInfoSSBO,
            sizeof(MeshInfo) * meshInfoBuffer.size(),
            (const void *)meshInfoBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );

    if(triangle_texbuff) glDeleteTextures(1, &triangle_texbuff);
    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
    triangle_texbuff = gen_buffer_texture(triangle_buff);
    lightidx_texbuff = gen_buffer_texture(lightidx_buff);
}

void Renderer::reload_bvhnodes(Scene *scene) {
    bvhNodeBuffer.resize(scene->sceneBVHRoot->siz);

    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->sceneBVHRoot, 0);
    int n = 0;
    while(!q.empty()) {
        auto [p, index] = q.front();
        q.pop();
        int il = -1, ir = -1;
        if(p->ls) il = ++n, q.emplace(p->ls, il);
        if(p->rs) ir = ++n, q.emplace(p->rs, ir);

        BVHNodeInfo y;
        y.aa = vec4(p->aabb.mi, 0);
        y.bb = vec4(p->aabb.mx, 0);
        y.lsIndex = il;
        y.rsIndex = ir;
        if(p->meshPtr) y.meshIndex = meshIndexMap[p->meshPtr];
        if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
        bvhNodeBuffer[index] = y;
    }
    assert(n + 1 == bvhNodeBuffer.size());

    if(bvhNodeSSBO) glDeleteBuffers(1, &bvhNodeSSBO);
    glCreateBuffers(1, &bvhNodeSSBO);
    glNamedBufferStorage(
            bvhNodeSSBO,
            sizeof(BVHNodeInfo) * bvhNodeBuffer.size(),
            (const void *)bvhNodeBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
}

void Renderer::reload_scene(Scene *scene) {
    reload_meshes(scene);
    reload_bvhnodes(scene);
}

void Renderer::draw() {
    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), light_num);
    glUniform1i(glGetUniformLocation(shaderProgram, "triangle_num"), triangle_num);
    bind_texture("triangles", triangle_texbuff, GL_TEXTURE_BUFFER);
    bind_texture("lightidxs", lightidx_texbuff, GL_TEXTURE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureHandleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, meshInfoSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, materialSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, bvhNodeSSBO);
    RenderPass::draw();
}

