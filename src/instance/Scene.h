//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_SCENE_H
#define PATH_TRACING_SCENE_H

#include "Instance.h"
#include "../BVH.h"

class Scene: public Instance{

    /**
     * recursive load meshes to world mesh
     * @param cur current instance
     * @param transform accumulated transform
     */
    void fetch_tree(Instance* cur, mat4 transform);
public:
    vector<Mesh> world_meshes;    /**< meshes in world space >**/
    BVHNode* bvh_root = nullptr;

    explicit Scene(const string &nm);

    /**
     * refatch all meshes an reload bvh
     */
    void reload();
};


#endif //PATH_TRACING_SCENE_H
