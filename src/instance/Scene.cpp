//
// Created by lx_tyin on 2023/2/20.
//

#include "Scene.h"
#include <iostream>

void Scene::fetch_tree(Instance *cur, mat4 transform) {
    for(auto *mesh: cur->meshes){
        Mesh nwm(*mesh);
        for(auto &t: nwm.triangles) {
            for(auto &v: t.vertex) v = transform * vec4(v, 1);
            for(auto &n: t.normal) n = glm::transpose(glm::inverse(transform)) * vec4(n, 0);
            t.center = (t.vertex[0] + t.vertex[1] + t.vertex[2]) / 3.0f;
        }
        world_meshes.push_back(nwm);
    }
    Instance *child;
    for(int i = 0; (child = cur->get_child(i)) != nullptr; i++){
        fetch_tree(child, transform * child->transform.matrix());
    }
}

void Scene::reload() {
    world_meshes.clear();
    fetch_tree(this, mat4(1));
    vector<Triangle*> tmp;
    for(auto &m: world_meshes)
        for(auto &t: m.triangles)
            tmp.push_back(&t);
    bvh_root = BVHNode::build(tmp);
    std::cout << "BVH size:" << bvh_root->siz << std::endl;
    std::cout << "BVH depth:" << bvh_root->depth << std::endl;
}

Scene::Scene(const string &nm): Instance(nm, nullptr) {}

