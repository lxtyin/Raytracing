//
// Created by lx_tyin on 2023/2/21.
//

#ifndef PATH_TRACING_BVH_H
#define PATH_TRACING_BVH_H

#include "instance/Triangle.h"
#include "AABB.h"
#include <vector>

class Mesh;

struct BVHPrimitive {
    AABB aabb;
    Mesh *meshPtr = nullptr;
    Triangle *trianglePtr = nullptr;
};

class BVHNode {
public:
    AABB aabb;
    BVHNode *ls = nullptr;
    BVHNode *rs = nullptr;
    Mesh* meshPtr = nullptr;
    Triangle* trianglePtr = nullptr;
    int siz = 1; // 子树节点数
    int depth = 0; // 树最大深度

    static BVHNode* build(std::vector<BVHPrimitive> &primitives);

    ~BVHNode();
};


#endif //PATH_TRACING_BVH_H
