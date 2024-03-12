//
// Created by lx_tyin on 2023/3/6.
//

#ifndef PATH_TRACING_MESH_H
#define PATH_TRACING_MESH_H

#include "Triangle.h"
#include "../material/Material.h"
#include "../BVH.h"
#include <vector>
#include <memory>

class Mesh {
public:
    string name = "A Mesh";
    std::vector<Triangle> triangles;    /**< triangles in local space >**/

    bool isEmitter = false;
    vec3 emission;

    BVHNode *meshBVHRoot;
    Material* material;        /**< material resource. */

    void build_meshBVH();

    Mesh(string _name, std::vector<Triangle> &&_triangles);
    Mesh() = delete;
    ~Mesh();
};


#endif //PATH_TRACING_MESH_H
