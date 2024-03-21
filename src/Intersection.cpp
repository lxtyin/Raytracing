//
// Created by 19450 on 2024/3/21.
//

#include "Intersection.h"


float Intersection::rayIntersectTriangle(Ray &ray, Triangle &tri) {
    vec3 E1 = tri.vertex[1] - tri.vertex[0];
    vec3 E2 = tri.vertex[2] - tri.vertex[0];
    vec3 S = ray.ori - tri.vertex[0];
    vec3 S1 = cross(ray.dir, E2);
    vec3 S2 = cross(S, E1);
    float k = 1.0 / dot(S1, E1);
    float t = dot(S2, E2) * k;
    float u = dot(S1, S) * k;
    float v = dot(S2, ray.dir) * k;

    bool exist = (0.001 < t && 0 < u && 0 < v && u + v < 1);

    if(!exist) return -1;
    else return t;
}

float Intersection::rayIntersectAABB(Ray &ray, AABB &aabb) {
    float tmi = 0, tmx = INF;
    vec3 t1 = (aabb.mi - ray.ori) / ray.dir;
    vec3 t2 = (aabb.mx - ray.ori) / ray.dir;
    tmi = max(tmi, min(t1.x, t2.x));
    tmi = max(tmi, min(t1.y, t2.y));
    tmi = max(tmi, min(t1.z, t2.z));
    tmx = min(tmx, max(t1.x, t2.x));
    tmx = min(tmx, max(t1.y, t2.y));
    tmx = min(tmx, max(t1.z, t2.z));

    if(tmi < tmx + 0.001) return tmi;
    else return -1;
}
