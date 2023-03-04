//
// Created by lx_tyin on 2023/2/20.
//

#include "Scene.h"

void Scene::update_triangles() {
    triangles.clear();
    for(auto *o: objects) {
        for(auto &t: o->triangles){
            Triangle y{};
            for(int i = 0;i < 3;i++) {
                y.vertex[i] = o->transform() * vec4(t.vertex[i], 1);
                // 法线变换 todo
//                    y.vertex[i] = o->transform() * vec4(y.normal[i], 0);
            }
            y.center = (y.vertex[0] + y.vertex[1] + y.vertex[2]) / 3.0f;
            triangles.push_back(y);
        }
    }
}

void Scene::reload() {
    update_triangles();
    vector<Triangle*> tmp;
    for(auto &t: triangles) tmp.push_back(&t);
    bvh_root = BVHNode::build(tmp);
}

void Scene::add(Object *obj) {
    objects.push_back(obj);
}

void Scene::add(const vector<Object *> &objs) {
    objects.insert(objects.end(), objs.begin(), objs.end());
}
