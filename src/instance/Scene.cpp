//
// Created by lx_tyin on 2023/2/20.
//

#include "Scene.h"
#include "Mesh.h"
#include "../BVH.h"
#include <iostream>

Scene* Scene::main_scene = nullptr;

Scene::Scene(const string &nm): Instance(nm, nullptr) {}

Scene::~Scene() {
    if(sceneBVHRoot) delete sceneBVHRoot;
}