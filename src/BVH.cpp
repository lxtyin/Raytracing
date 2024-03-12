//
// Created by lx_tyin on 2023/2/21.
//

#include "BVH.h"
#include <algorithm>
#include <functional>
#include "instance/Mesh.h"

BVHNode* BVHNode::build(std::vector<BVHPrimitive> &primitives) {
    BVHNode* cur = new BVHNode;

    int n = primitives.size();
    assert(n > 0);
    if(n == 1) {
        BVHPrimitive p = primitives[0];
        if(p.meshPtr) {
            assert(p.meshPtr->meshBVHRoot);
            delete cur;
            return p.meshPtr->meshBVHRoot;
        } else {
            assert(p.trianglePtr);
            cur->trianglePtr = p.trianglePtr;
            cur->aabb = p.aabb;
            cur->depth = 1;
            cur->siz = 1;
            return cur;
        }
    }

    for(BVHPrimitive &p: primitives) {
        cur->aabb.combine(p.aabb);
    }

    std::vector<BVHPrimitive> t[3] = {primitives, primitives, primitives};
    std::vector<AABB> pre(primitives.size()), suf(primitives.size());

    int bs_d = 0, bs_i = 0;
    float bs_cost = n * cur->aabb.surface();
    for(int d = 0;d < 3;d++) {
        std::sort(t[d].begin(), t[d].end(), [&](BVHPrimitive x, BVHPrimitive y) {
            return x.aabb.center()[d] < y.aabb.center()[d];
        });
        pre[0] = AABB();
        for(int i = 0;i < n;i++){
            if(i != 0) pre[i] = pre[i - 1];
            pre[i].combine(t[d][i].aabb);
        }
        suf[n - 1] = AABB();
        for(int i = n - 1;i >= 0;i--){
            if(i != n - 1) suf[i] = suf[i + 1];
            suf[i].combine(t[d][i].aabb);
        }

        for(int i = 0;i < n - 1;i++) {
            float c = pre[i].surface() * (i + 1) + suf[i + 1].surface() * (n - i - 1);
            if(c < bs_cost) {
                bs_cost = c;
                bs_d = d;
                bs_i = i;
            }
        }
    }
    assert(bs_i < n);

    std::vector<BVHPrimitive> rt;
    while(t[bs_d].size() > bs_i + 1){
        rt.push_back(t[bs_d].back());
        t[bs_d].pop_back();
    }

    cur->ls = build(t[bs_d]);
    cur->rs = build(rt);
    cur->siz = cur->ls->siz + cur->rs->siz + 1;
    cur->depth = max(cur->ls->depth, cur->rs->depth) + 1;
    return cur;
}

BVHNode::~BVHNode() {
    if(ls && !ls->meshPtr) delete ls;
    if(rs && !rs->meshPtr) delete rs;
}
