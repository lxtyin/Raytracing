//
// Created by 19450 on 2024/3/5.
//

#ifndef PATH_TRACING_AABB_H
#define PATH_TRACING_AABB_H

#include "glm/vec3.hpp"
#include "glm/matrix.hpp"
#include <vector>
using glm::vec3;
using std::max;
using std::min;

const float INF = 1e18;

class AABB {
    vec3 vmin(const vec3 &x, const vec3 &y);
    vec3 vmax(const vec3 &x, const vec3 &y);
public:
    vec3 mi, mx;

    AABB();

    void combine(const AABB &t);

    void addPoint(const vec3 &p);

    vec3 center() const;

    float surface() const;
};


#endif //PATH_TRACING_AABB_H
