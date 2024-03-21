//
// Created by lx_tyin on 2023/2/20.
//

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/gtc/type_ptr.hpp"
#include "../Config.h"
#include "Renderer.h"
#include "../tool/tool.h"
#include <fstream>
#include <queue>

void Renderer::reload_meshInfos(Scene* scene) {
    textureHandlesBuffer.clear();
    materialBuffer.clear();
    meshInfoBuffer.clear();

    std::map<Texture*, uint> textureIndexMap;

    for(auto &[u, mat]: scene->allMeshes) {
        assert(u->material);
        auto umtexs = u->material->textures();
        for(Texture *t: umtexs) {
            if(!textureIndexMap.count(t)) {
                textureIndexMap[t] = textureHandlesBuffer.size();
                textureHandlesBuffer.push_back(t->textureHandle);
            }
        }
        int materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);
        MeshInfo y;
        y.world2local = glm::inverse(mat);
        y.emission = vec4(u->emission, u->isEmitter ? 1 : -1);
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
}

void Renderer::reload_triangles(Scene *scene) {
    // update triangles
    triangleBuffer.clear();
    std::map<Triangle*, uint> triangleIndexMap;

    for(auto &[u, mat]: scene->allMeshes) {
        for(auto &t: u->triangles) {
            triangleIndexMap[&t] = triangleBuffer.size();
            triangleBuffer.push_back(t);
        }
    }
    if(triangleSSBO) glDeleteBuffers(1, &triangleSSBO);
    glCreateBuffers(1, &triangleSSBO);
    glNamedBufferStorage(
            triangleSSBO,
            sizeof(Triangle) * triangleBuffer.size(),
            (const void *)triangleBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );

    // update meshBVH
    int meshBVHsize = 0;
    for(auto &[u, mat]: scene->allMeshes) meshBVHsize += u->meshBVHRoot->siz;
    meshBVHBuffer.resize(meshBVHsize);

    // 0~M-1 节点作为各个meshBVH的根节点，index同meshIndex
    int N = (int)scene->allMeshes.size() - 1;
    for(int i = 0;i < scene->allMeshes.size();i++) {
        std::queue<std::pair<BVHNode*, int>> q; // <node, index>
        q.emplace(scene->allMeshes[i].first->meshBVHRoot, i);
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
//            if(p->meshPtr) y.meshIndex = meshIndexMap[p->meshPtr];
            if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
            meshBVHBuffer[index] = y;
        }
    }
    if(meshBVHSSBO) glDeleteBuffers(1, &meshBVHSSBO);
    glCreateBuffers(1, &meshBVHSSBO);
    glNamedBufferStorage(
            meshBVHSSBO,
            sizeof(BVHNodeInfo) * meshBVHBuffer.size(),
            (const void *)meshBVHBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
}

void Renderer::reload_sceneBVH(Scene *scene) {
    std::map<Mesh*, uint> meshIndexMap;
    for(int i = 0;i < scene->allMeshes.size();i++) meshIndexMap[scene->allMeshes[i].first] = i;

    sceneBVHBuffer.resize(scene->sceneBVHRoot->siz - meshBVHBuffer.size());
    std::queue<std::pair<BVHNode*, int>> q; // <node, index>
    q.emplace(scene->sceneBVHRoot, 0);
    int N = 0;
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
//        if(p->meshPtr) y.meshIndex = meshIndexMap[p->meshPtr];
//        if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
        sceneBVHBuffer[index] = y;
    }
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
    reload_meshInfos(scene);
    reload_triangles(scene);
    reload_sceneBVH(scene);
}

void Renderer::reload_sceneinfos(Scene *scene) {
    reload_meshInfos(scene);
    reload_sceneBVH(scene);
}

void Renderer::draw(GBuffer &gbuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, textureHandleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, materialSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, triangleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, meshInfoSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, meshBVHSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, sceneBVHSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, gbuffer.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, gbuffer.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, gbuffer.depthGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, gbuffer.motionGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, gbuffer.albedoGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, gbuffer.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, gbuffer.meshIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, gbuffer.numSamplesGBufferSSBO);


    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    // Run compute shader
//    glDispatchCompute((SCREEN_H + 31) / 32,
//                      (SCREEN_W + 31) / 32,
//                      1);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

//    cout << colorGBufferSSBO << "R: ";
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorGBufferSSBO);
//    float* tmpdata = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, framesize * sizeof(float), GL_MAP_READ_BIT);
//    for(int i = 0;i < 10;i++) std::cout << tmpdata[i] << ' ';
//    std::cout << std::endl;
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

Renderer::Renderer(const string &shaderPath): VertexFragmentRenderPass(shaderPath) {}
