//
// Created by lx_tyin on 2023/2/21.
//

#include "BVH.h"
#include <algorithm>
#include <functional>
using std::max;
using std::min;

const float INF = 1e18;

vec3 vmin(const vec3 &x, const vec3 &y) {
    return {min(x[0], y[0]), min(x[1], y[1]), min(x[2], y[2])};
}
vec3 vmax(const vec3 &x, const vec3 &y) {
    return {max(x[0], y[0]), max(x[1], y[1]), max(x[2], y[2])};
}
float cost(const vec3 &aa, const vec3 &bb) {
    vec3 t = bb - aa;
    return t.x * t.y + t.y * t.z + t.z * t.x;
}
vec3 pre_aa[MAX_TRIANGLES], pre_bb[MAX_TRIANGLES], suf_aa[MAX_TRIANGLES], suf_bb[MAX_TRIANGLES];

BVHNode* BVHNode::build(vector<Triangle*> &triangles) {

    int n = triangles.size();
    BVHNode* cur = new BVHNode;

    cur->aa = vec3(INF);
    cur->bb = vec3(-INF);
    for(auto *t: triangles){
        for(auto & v : t->vertex) {
            cur->aa = vmin(cur->aa, v);
            cur->bb = vmax(cur->bb, v);
        }
    }
    assert(n > 0);
    if(n == 1) {
        cur->isleaf = true;
        cur->triangle = triangles[0];
        cur->siz = 1;
        return cur;
    }

    vector<Triangle*> t[3] = {triangles, triangles, triangles};

    int bs_d = 0, bs_i = 0;
    float bs_cost = 2 * cost(cur->aa, cur->bb); // max possible cost
    for(int d = 0;d < 3;d++) {
        std::sort(t[d].begin(), t[d].end(), [&](Triangle *x, Triangle *y) {
            return x->center[d] < y->center[d];
        });
        pre_aa[0] = vec3(INF);
        pre_bb[0] = vec3(-INF);
        for(int i = 0;i < n;i++){
            if(i != 0){
                pre_aa[i] = pre_aa[i - 1];
                pre_bb[i] = pre_bb[i - 1];
            }
            for(auto &v: t[d][i]->vertex) {
                pre_aa[i] = vmin(pre_aa[i], v);
                pre_bb[i] = vmax(pre_bb[i], v);
            }
        }

        suf_aa[n - 1] = vec3(INF);
        suf_bb[n - 1] = vec3(-INF);
        for(int i = n - 1;i >= 0;i--){
            if(i != n - 1){
                suf_aa[i] = suf_aa[i + 1];
                suf_bb[i] = suf_bb[i + 1];
            }
            for(auto &v: t[d][i]->vertex) {
                suf_aa[i] = vmin(suf_aa[i], v);
                suf_bb[i] = vmax(suf_bb[i], v);
            }
        }

        for(int i = 0;i < n - 1;i++) {
            float c = cost(pre_aa[i], pre_bb[i]) * (i + 1) + cost(suf_aa[i + 1], suf_bb[i + 1]) * (n - i - 1);
            if(c < bs_cost) {
                bs_cost = c;
                bs_d = d;
                bs_i = i;
            }
        }
    }
    assert(bs_i < n);

    vector<Triangle*> rt;
    while(t[bs_d].size() > bs_i + 1){
        rt.push_back(t[bs_d].back());
        t[bs_d].pop_back();
    }

    cur->ls = build(t[bs_d]);
    cur->rs = build(rt);
    cur->siz = cur->ls->siz + cur->rs->siz + 1;
    return cur;
}