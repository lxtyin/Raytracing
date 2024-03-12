//
// Created by lx_tyin on 2023/2/20.
//

#include "Renderer.h"
#include "tool/tool.h"
#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include <fstream>
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


void Renderer::reload_materials(Scene *scene) {

}

void Renderer::reload_transforms(Scene *scene) {
    // TODO: 拆分为两个update level
    textureHandlesBuffer.clear();
    materialBuffer.clear();
    meshInfoBuffer.clear();
    meshIndexMap.clear();

    targetMeshes.clear();
    scene->fetch_meshes(scene, mat4(1), targetMeshes);
    std::map<Texture*, uint> textureIndexMap;

    lightidx_buff.clear();
    light_num = 0;
    for(auto &[u, mat]: targetMeshes) {
        assert(u->material);
        auto umtexs = u->material->textures();
        for(Texture *t: umtexs) {
            if(!textureIndexMap.count(t)) {
                textureIndexMap[t] = textureHandlesBuffer.size();
                textureHandlesBuffer.push_back(t->textureHandle);
            }
        }
        int materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);

        assert(!meshIndexMap.count(u));
        meshIndexMap[u] = meshInfoBuffer.size();
        MeshInfo y;
        y.world2local = glm::inverse(mat);
        y.emission = vec4(u->emission, u->isEmitter ? 1 : -1);// TODO Emission
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

    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
    lightidx_texbuff = gen_buffer_texture(lightidx_buff);
}


void Renderer::reload_meshes(Scene *scene) {
    // TODO: 拆分为两个update level
    textureHandlesBuffer.clear();
    materialBuffer.clear();
    triangleBuffer.clear();
    meshInfoBuffer.clear();
    meshIndexMap.clear();
    triangleIndexMap.clear();

    targetMeshes.clear();
    scene->fetch_meshes(scene, mat4(1), targetMeshes);
    std::map<Texture*, uint> textureIndexMap;

    lightidx_buff.clear();
    light_num = 0;
    for(auto &[u, mat]: targetMeshes) {
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
            triangleIndexMap[&t] = triangleBuffer.size();
            triangleBuffer.push_back(t);
        }

        assert(!meshIndexMap.count(u));
        meshIndexMap[u] = meshInfoBuffer.size();
        MeshInfo y;
        y.world2local = glm::inverse(mat);
        y.emission = vec4(u->emission, u->isEmitter ? 1 : -1);// TODO Emission
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

    if(triangleSSBO) glDeleteBuffers(1, &triangleSSBO);
    glCreateBuffers(1, &triangleSSBO);
    glNamedBufferStorage(
            triangleSSBO,
            sizeof(Triangle) * triangleBuffer.size(),
            (const void *)triangleBuffer.data(),
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

    if(lightidx_texbuff) glDeleteTextures(1, &lightidx_texbuff);
    lightidx_texbuff = gen_buffer_texture(lightidx_buff);
}

void Renderer::reload_bvhnodes(Scene *scene) {

    int meshBVHsize = 0;
    for(auto &[u, mat]: targetMeshes) meshBVHsize += u->meshBVHRoot->siz;
    meshBVHBuffer.resize(meshBVHsize);

    // 0-M个节点作为各个meshBVH的根节点，index同meshIndex

    int N = (int)targetMeshes.size() - 1;
    // load all bvh of meshes;
    for(int i = 0;i < targetMeshes.size();i++) {
        std::queue<std::pair<BVHNode*, int>> q; // <node, index>
        q.emplace(targetMeshes[i].first->meshBVHRoot, i);
        while(!q.empty()) {
            auto [p, index] = q.front();
            q.pop();
            int il = -1, ir = -1;
            if(p->ls) il = ++N, q.emplace(p->ls, il);
            if(p->rs) ir = ++N, q.emplace(p->rs, ir);

            BVHNodeInfo y;
            y.aa = vec4(p->aabb.mi, 0);
            y.bb = vec4(p->aabb.mx, 0);
            y.lsIndex = il;
            y.rsIndex = ir;
            if(p->meshPtr) y.meshIndex = meshIndexMap[p->meshPtr];
            if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
            meshBVHBuffer[index] = y;
        }
    }

    sceneBVHBuffer.resize(scene->sceneBVHRoot->siz - meshBVHsize);
    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->sceneBVHRoot, 0);
    N = 0;
    while(!q.empty()) {
        auto [p, index] = q.front();
        q.pop();
        int il = -1, ir = -1;
        if(p->ls) {
            if(p->ls->meshPtr) il = -meshIndexMap[p->ls->meshPtr];
            else il = ++N, q.emplace(p->ls, il);
        }
        if(p->rs) {
            if(p->rs->meshPtr) ir = -meshIndexMap[p->rs->meshPtr];
            else ir = ++N, q.emplace(p->rs, ir);
        }
        BVHNodeInfo y;
        y.aa = vec4(p->aabb.mi, 0);
        y.bb = vec4(p->aabb.mx, 0);
        y.lsIndex = il;
        y.rsIndex = ir;
        if(p->meshPtr) y.meshIndex = meshIndexMap[p->meshPtr];
        if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
        sceneBVHBuffer[index] = y;
    }

    if(meshBVHSSBO) glDeleteBuffers(1, &meshBVHSSBO);
    glCreateBuffers(1, &meshBVHSSBO);
    glNamedBufferStorage(
            meshBVHSSBO,
            sizeof(BVHNodeInfo) * meshBVHBuffer.size(),
            (const void *)meshBVHBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
    if(sceneBVHSSBO) glDeleteBuffers(1, &sceneBVHSSBO);
    glCreateBuffers(1, &sceneBVHSSBO);
    glNamedBufferStorage(
            sceneBVHSSBO,
            sizeof(BVHNodeInfo) * sceneBVHBuffer.size(),
            (const void *)sceneBVHBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
}

void Renderer::reload_scene(Scene *scene) {
    reload_meshes(scene);
    reload_bvhnodes(scene);
}

void Renderer::draw() {
    glUniform1i(glGetUniformLocation(shaderProgram, "light_t_num"), light_num);
    bind_texture("lightidxs", lightidx_texbuff, GL_TEXTURE_BUFFER);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureHandleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, meshInfoSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, meshBVHSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, sceneBVHSSBO);
    RenderPass::draw();
}

Renderer::Renderer(const string &frag_shader_path, int attach_num, bool to_screen)
: RenderPass(frag_shader_path, attach_num, to_screen) { }
