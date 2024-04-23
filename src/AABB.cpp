//
// Created by lx_tyin on 2023/2/21.
//

#include "AABB.h"


vec3 AABB::vmin(const vec3 &x, const vec3 &y) {
    return {min(x[0], y[0]), min(x[1], y[1]), min(x[2], y[2])};
}

vec3 AABB::vmax(const vec3 &x, const vec3 &y) {
    return {max(x[0], y[0]), max(x[1], y[1]), max(x[2], y[2])};
}

AABB::AABB(): mi(INF), mx(-INF) {}

void AABB::combine(const AABB &t) {
    mi = vmin(mi, t.mi);
    mx = vmax(mx, t.mx);
}

void AABB::addPoint(const vec3 &p) {
    mi = vmin(mi, p);
    mx = vmax(mx, p);
}

vec3 AABB::center() const {
    return (mi + mx) * 0.5f;
}

float AABB::surface() const {
    vec3 t = mx - mi;
    return t.x * t.y + t.y * t.z + t.z * t.x;
}
