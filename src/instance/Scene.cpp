//
// Created by lx_tyin on 2023/2/20.
//

#include "Scene.h"
#include <iostream>


void Scene::fetch_meshes(Instance* cur, mat4 transform2world, std::vector<std::pair<Mesh*, mat4>> &allMeshes) {
    if(cur->mesh) {
        allMeshes.emplace_back(std::make_pair(cur->mesh, transform2world));
    }
    Instance *child;
    for(int i = 0; (child = cur->get_child(i)) != nullptr; i++){
        fetch_meshes(child, transform2world * child->transform.matrix(), allMeshes);
    }
}


void Scene::build_sceneBVH() {
    std::vector<BVHPrimitive> primitives;
    for(auto &[u, mat]: allMeshes){
        BVHPrimitive p;
        p.meshPtr = u;
        AABB cube = u->meshBVHRoot->aabb;
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
    if(sceneBVHRoot) delete sceneBVHRoot;
    sceneBVHRoot = BVHNode::build(primitives);
}


void Scene::update() {
    allMeshes.clear();
    fetch_meshes(this, mat4(1), allMeshes);
    build_sceneBVH();
}

Scene::Scene(const string &nm): Instance(nm, nullptr) {}

Scene::~Scene() {
    if(sceneBVHRoot) delete sceneBVHRoot;
}