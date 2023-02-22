//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_SCENE_H
#define PATH_TRACING_SCENE_H

#include "Object.h"
#include "BVH.h"

class Scene {

    /// 将所有三角形转换到世界坐标系下，存储在triangles里面
    void update_triangles();
public:
    vector<Object*> objects;
    vector<Triangle> triangles; // 在世界坐标系下表示所有三角形
    BVHNode* bvh_root;

    /// 场景更新时调用
    void reload();
};


#endif //PATH_TRACING_SCENE_H
