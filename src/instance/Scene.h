//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_SCENE_H
#define PATH_TRACING_SCENE_H

#include "Instance.h"

class BVHNode;

class Scene: public Instance{

public:
    static Scene *main_scene;

    BVHNode* sceneBVHRoot = nullptr;

    explicit Scene(const string &nm);

    ~Scene();
};

#endif //PATH_TRACING_SCENE_H