//
// Created by lx_tyin on 2023/2/21.
//

#ifndef PATH_TRACING_BVH_H
#define PATH_TRACING_BVH_H

#include "instance/Triangle.h"
#include "AABB.h"
#include <vector>
#include "Intersection.h"

class Instance;

struct BVHPrimitive {
    AABB aabb;
    Instance *instancePtr = nullptr;
    Triangle *trianglePtr = nullptr;
};

/**
 * Two-level BVH Tree.
 * In scene-level, each mesh (in world space) is considered as a primitive (a box).
 * At node has instancePtr != nullptr, only ls point to its object-level BVH tree,
 * you need to transform the ray into the mesh's loacl space,
 * and continue to intersect in object-level.
 */
class BVHNode {
public:
    AABB aabb;
    BVHNode *ls = nullptr;
    BVHNode *rs = nullptr;
    Triangle* trianglePtr = nullptr;
    Instance* instancePtr = nullptr;

    int siz = 1; // 子树节点数
    int depth = 0; // 树最大深度

    static BVHNode* build(std::vector<BVHPrimitive> &primitives);

    void rayIntersect(Ray ray, Intersection &isect, Instance* insp = nullptr);

    ~BVHNode();
};


#endif //PATH_TRACING_BVH_H
