//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_SCENE_H
#define PATH_TRACING_SCENE_H

#include "Instance.h"
#include "../BVH.h"


class Scene: public Instance{

    /**
     * recursive load all meshes and get their transform2world.
     */
    void fetch_meshes(Instance* cur, mat4 transform2world, std::vector<std::pair<Mesh*, mat4>> &allMeshes);

    void build_sceneBVH();

public:

    /**
     * All the meshes and it's matrix to global. This array is used for final rendering.
     */
    std::vector<std::pair<Mesh*, mat4>> allMeshes;

    BVHNode* sceneBVHRoot = nullptr;

    explicit Scene(const string &nm);

    /**
     * update allMeshes and BVH.
     */
    void update();
};

#endif //PATH_TRACING_SCENE_H