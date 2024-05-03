//
// Created by lx_tyin on 2023/3/6.
//

#ifndef PATH_TRACING_MESH_H
#define PATH_TRACING_MESH_H

#include "glad/glad.h"
#include "Triangle.h"
#include <vector>
#include <memory>
#include <string>
using std::string;

class BVHNode;

class Mesh {
    GLuint VAO, VBO;
    void bind_vaovbo();

public:
    static int triangleCount;

    string name = "A Mesh";
    std::vector<Triangle> triangles;    /**< triangles in local space >**/

    BVHNode *meshBVHRoot;

    void build_meshBVH();

    void draw_in_rasterization();

    Mesh(string _name, std::vector<Triangle> &&_triangles);
    Mesh() = delete;
    ~Mesh();
};


#endif //PATH_TRACING_MESH_H
