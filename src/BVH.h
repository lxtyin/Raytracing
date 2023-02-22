//
// Created by lx_tyin on 2023/2/21.
//

#ifndef PATH_TRACING_BVH_H
#define PATH_TRACING_BVH_H

#include "Triangle.h"
#include <vector>
using std::vector;

class BVHNode {
public:
    vec3 aa, bb;            // x(a[0], b[0]), y(a[1], b[1]), z(a[2], b[2])
    bool isleaf = false;
    BVHNode *ls = nullptr;
    BVHNode *rs = nullptr;
    Triangle *triangle = nullptr;
    int siz; // 子树节点数

    static BVHNode* build(vector<Triangle*> &triangles);
};


#endif //PATH_TRACING_BVH_H