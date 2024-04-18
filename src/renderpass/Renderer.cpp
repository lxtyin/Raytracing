//
// Created by lx_tyin on 2023/2/20.
//

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "../Config.h"
#include "Renderer.h"
#include "../texture/Texture.h"
#include "../instance/Mesh.h"
#include "../instance/Scene.h"
#include "../material/Material.h"
#include "../BVH.h"
#include <queue>
#include <set>

void Renderer::reload_meshInfos(Scene* scene) {
    textureHandlesBuffer.clear();
    materialBuffer.clear();
    instanceInfoBuffer.clear();

    std::map<Texture*, uint> textureIndexMap;

    for(auto &[u, mat]: scene->allMeshes) {
        assert(u->mesh);
        assert(u->material);
        auto umtexs = u->material->textures();
        for(Texture *t: umtexs) {
            if(!textureIndexMap.count(t)) {
                textureIndexMap[t] = textureHandlesBuffer.size();
                textureHandlesBuffer.push_back(t->textureHandle);
            }
        }
        int materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);
        InstanceInfo y;
        y.world2local = glm::inverse(mat);
        y.materialPtr = materialPtr;
        instanceInfoBuffer.push_back(y);
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
    if(instanceInfoSSBO) glDeleteBuffers(1, &instanceInfoSSBO);
    glCreateBuffers(1, &instanceInfoSSBO);
    glNamedBufferStorage(
            instanceInfoSSBO,
            sizeof(InstanceInfo) * instanceInfoBuffer.size(),
            (const void *)instanceInfoBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
}

void Renderer::reload_triangles(Scene *scene) {
    // update triangles
    triangleBuffer.clear();
    std::map<Triangle*, uint> triangleIndexMap;
    std::set<Mesh*> meshes;

    for(auto &[u, mat]: scene->allMeshes) {
        if(!meshes.count(u->mesh)) {
            meshes.insert(u->mesh);
            for(auto &t: u->mesh->triangles) {
                triangleIndexMap[&t] = triangleBuffer.size();
                triangleBuffer.push_back(t);
            }
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
    meshBVHBuffer.clear();

    // 0 ~ numMeshes-1 节点作为各个meshBVH的根节点
    std::queue<BVHNode*> q; // <node, index>
    for(auto mesh: meshes) {
        q.push(mesh->meshBVHRoot);
    }

    int N = meshes.size();
    while(!q.empty()) {
        BVHNode* p = q.front();
        q.pop();
        int il = -1, ir = -1;
        if(p->ls) il = N++, q.emplace(p->ls);
        if(p->rs) ir = N++, q.emplace(p->rs);

        BVHNodeInfo y;
        y.aa = vec4(p->aabb.mi, 0);
        y.bb = vec4(p->aabb.mx, 0);
        y.lsIndex = il;
        y.rsIndex = ir;
        if(p->trianglePtr) y.triangleIndex = triangleIndexMap[p->trianglePtr];
        meshBVHBuffer.push_back(y);
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
    std::set<Mesh*> meshes;
    std::map<Mesh*, uint> meshIndexMap;
    instanceIndexMap.clear();

    for(int i = 0;i < scene->allMeshes.size();i++) {
        Instance* ist = scene->allMeshes[i].first;
        instanceIndexMap[ist] = i;
        if(!meshes.count(ist->mesh)) {
            meshes.insert(ist->mesh);
        }
    }
    int numMeshes = 0;
    for(auto mesh: meshes) {
        meshIndexMap[mesh] = numMeshes++;
    }

    sceneBVHBuffer.clear();

    std::queue<BVHNode*> q; // <node, index>
    q.emplace(scene->sceneBVHRoot);
    int N = 0;
    while(!q.empty()) {
        BVHNode* p = q.front();
        q.pop();
        if(p->instancePtr) {
            BVHNodeInfo y;
            y.aa = vec4(p->aabb.mi, 0);
            y.bb = vec4(p->aabb.mx, 0);
            y.instanceIndex = instanceIndexMap[p->instancePtr];
            y.lsIndex = meshIndexMap[p->instancePtr->mesh]; // link to meshBVHBuffer
            sceneBVHBuffer.push_back(y);
            continue;
        }

        int il = -1, ir = -1;
        if(p->ls) il = ++N, q.emplace(p->ls);
        if(p->rs) ir = ++N, q.emplace(p->rs);

        BVHNodeInfo y;
        y.aa = vec4(p->aabb.mi, 0);
        y.bb = vec4(p->aabb.mx, 0);
        y.lsIndex = il;
        y.rsIndex = ir;
        sceneBVHBuffer.push_back(y);
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
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, instanceInfoSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, meshBVHSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, sceneBVHSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, gbuffer.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, gbuffer.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, gbuffer.depthGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, gbuffer.motionGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, gbuffer.albedoGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, gbuffer.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, gbuffer.instanceIndexGBufferSSBO);
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


int Renderer::query_instanceIndex(Instance *instance) {
    if(!instanceIndexMap.count(instance)) return -1;
    return instanceIndexMap[instance];
}
