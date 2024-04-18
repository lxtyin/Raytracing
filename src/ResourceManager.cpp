//
// Created by 19450 on 2024/3/4.
//

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "ResourceManager.h"
#include "Config.h"
#include "tool/tool.h"
#include "material/Material.h"
#include "instance/Scene.h"
#include "instance/Mesh.h"
#include "BVH.h"
#include <iostream>
#include <queue>
#include <set>
#include "glad/glad.h"
#include "glfw/glfw3.h"

ResourceManager* ResourceManager::manager = nullptr;

void ResourceManager::add_mesh(Mesh *y) {
    assert(!meshIndexMap.count(y));
    meshIndexMap[y] = (int)meshes.size();
    meshes.push_back(y);
    reload_meshes();
}
void ResourceManager::del_mesh(Mesh *y) {
    assert(meshIndexMap.count(y));
    meshes[meshIndexMap[y]] = nullptr;
    meshIndexMap[y] = 0;
    reload_meshes();
}
void ResourceManager::add_texture(Texture *y) {
    assert(!textureIndexMap.count(y));
    textureIndexMap[y] = (int)textures.size();
    textures.push_back(y);
    reload_textures();
}
void ResourceManager::del_texture(Texture *y) {
    assert(textureIndexMap.count(y));
    textures[textureIndexMap[y]] = nullptr;
    textureIndexMap[y] = 0;
    reload_textures();
}

void ResourceManager::update_globalinstance(Scene *scene) {
    globalInstances.clear();
    instanceIndexMap.clear();
    update_globalinstance_recursive(scene, mat4(1.0));
}

void ResourceManager::update_globalinstance_recursive(Instance *cur, mat4 transform2world) {
    if(cur->mesh) {
        instanceIndexMap[cur] = (int)globalInstances.size();
        globalInstances.emplace_back(cur, transform2world);
    }
    Instance *child;
    for(int i = 0; (child = cur->get_child(i)) != nullptr; i++){
        update_globalinstance_recursive(child, transform2world * child->transform.matrix());
    }
}


void ResourceManager::reload_textures() {
    auto tmp = textures;
    textures.clear();
    textureIndexMap.clear();
    for(Texture *t: tmp) if(t) {
        textureIndexMap[t] = textures.size();
        textures.push_back(t);
    }

    textureHandlesBuffer.clear();
    for(Texture *t: textures) if(t) textureHandlesBuffer.push_back(t->textureHandle);

    if(textureHandleSSBO) glDeleteBuffers(1, &textureHandleSSBO);
    glCreateBuffers(1, &textureHandleSSBO);
    glNamedBufferStorage(
            textureHandleSSBO,
            sizeof(GLuint64) * textureHandlesBuffer.size(),
            (const void *)textureHandlesBuffer.data(),
            GL_DYNAMIC_STORAGE_BIT
    );
}

void ResourceManager::reload_meshes() {
    // update triangles
    triangleBuffer.clear();
    triangleIndexMap.clear();
    meshIndexMap.clear();

    auto tmpmeshes = meshes;
    meshes.clear();
    for(auto *u: tmpmeshes) {
        if(!u) continue;
        meshIndexMap[u] = meshes.size();
        meshes.push_back(u);
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

void ResourceManager::reload_instances() {
    materialBuffer.clear();
    instanceInfoBuffer.clear();
    for(auto &[u, mat]: globalInstances) {
        assert(u->mesh);
        assert(u->material);
        int materialPtr = u->material->insert_buffer(materialBuffer, textureIndexMap);
        InstanceInfo y;
        y.world2local = glm::inverse(mat);
        y.materialPtr = materialPtr;
        y.meshIndex = meshIndexMap[u->mesh];
        instanceInfoBuffer.push_back(y);
    }
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

void ResourceManager::reload_sceneBVH(BVHNode *root) {
    sceneBVHBuffer.clear();

    std::queue<BVHNode*> q; // <node, index>
    q.emplace(root);
    int N = 0;
    while(!q.empty()) {
        BVHNode* p = q.front();
        q.pop();
        if(p->instancePtr) {
            BVHNodeInfo y;
            y.aa = vec4(p->aabb.mi, 0);
            y.bb = vec4(p->aabb.mx, 0);
            y.instanceIndex = instanceIndexMap[p->instancePtr];
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

void ResourceManager::reload_scene(Scene *scene) {
    update_globalinstance(scene);

    std::vector<BVHPrimitive> primitives;
    for(auto &[u, mat]: globalInstances){
        BVHPrimitive p;
        p.instancePtr = u;
        AABB cube = u->mesh->meshBVHRoot->aabb;
        for(int i = 0;i < 8;i++) {
            vec3 point = {
                    (i & 1) ? cube.mi[0] : cube.mx[0],
                    (i & 2) ? cube.mi[1] : cube.mx[1],
                    (i & 4) ? cube.mi[2] : cube.mx[2]
            };
            point = mat * vec4(point, 1);
            p.aabb.addPoint(point);
        }
        primitives.push_back(p);
    }
    if(scene->sceneBVHRoot) delete scene->sceneBVHRoot;
    scene->sceneBVHRoot = BVHNode::build(primitives);

    reload_instances();
    reload_sceneBVH(scene->sceneBVHRoot);
}

int ResourceManager::queryInstanceIndex(Instance *ins) {
    if(!instanceIndexMap.count(ins)) return -1;
    return instanceIndexMap[ins];
}

ResourceManager::ResourceManager() {
    textureHandleSSBO = materialSSBO = triangleSSBO = instanceInfoSSBO = meshBVHSSBO = sceneBVHSSBO = 0;
}


