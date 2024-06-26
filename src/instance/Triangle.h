//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_TRIANGLE_H
#define PATH_TRACING_TRIANGLE_H

#include "glm/vec3.hpp"
#include "glm/matrix.hpp"
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

class Triangle {
public:
    vec3 vertex[3];
    vec3 normal[3];     /**< normal for each vertex >**/
    vec2 uv[3];

    Triangle() = default;
    Triangle(vec3 v0, vec3 v1, vec3 v2,
             vec3 n0, vec3 n1, vec3 n2,
             vec2 u0 = vec2(0), vec2 u1 = vec2(0), vec2 u2 = vec2(0)):
            vertex{v0, v1, v2}, normal{n0, n1, n2}, uv{u0, u1, u2} {
    }
};


#endif //PATH_TRACING_TRIANGLE_H
