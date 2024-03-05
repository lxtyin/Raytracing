//
// Created by lx_tyin on 2023/2/20.
//

#include "Scene.h"
#include <iostream>


void Scene::fetch_meshes(Instance* cur, mat4 transform2world, std::vector<std::pair<Mesh*, mat4>> &allMeshes) {
    for(auto *m: cur->meshes) {
        allMeshes.emplace_back(std::make_pair(m, transform2world));
    }
    Instance *child;
    for(int i = 0; (child = cur->get_child(i)) != nullptr; i++){
        fetch_meshes(child, transform2world * child->transform.matrix(), allMeshes);
    }
}

void Scene::reload() {

    std::vector<std::pair<Mesh*, mat4>> allMeshes;
    fetch_meshes(this, mat4(1), allMeshes);

    std::vector<Triangle*> tmp;
    for(auto &[u, mat]: allMeshes){
        for(auto &t: u->triangles) {
            tmp.push_back(&t);
            // TODO transform
        }
    }

    bvh_root = BVHNode::build(tmp);
    std::cout << "BVH size:" << bvh_root->siz << std::endl;
    std::cout << "BVH depth:" << bvh_root->depth << std::endl;
}

Scene::Scene(const string &nm): Instance(nm, nullptr) {}

