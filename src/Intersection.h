//
// Created by 19450 on 2024/3/21.
//

#ifndef PATH_TRACING_INTERSECTION_H
#define PATH_TRACING_INTERSECTION_H

#include "glad/glad.h"
#include "instance/Triangle.h"
#include "instance/Instance.h"
#include "AABB.h"

struct Ray {
    vec3 ori, dir;
    Ray(vec3 &_o, vec3 &_d): ori(_o), dir(_d) {}
};

/**
 * Could be expanded, now it's only used for cursor selection
 */
class Intersection {
public:
    /**
     * Basic intersect event
     * \return time, if not exist, return -1;
     */
    static bool rayIntersectTriangle(Ray &ray, Triangle &triangle, float &tnear);
    static bool rayIntersectAABB(Ray &ray, AABB &aabb, float &tnear);

    bool exist = false;
    float t = INF;
    Instance *instancePtr = nullptr;
    Triangle *trianglePtr = nullptr;
};


#endif //PATH_TRACING_INTERSECTION_H
