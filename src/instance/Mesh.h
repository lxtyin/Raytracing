//
// Created by lx_tyin on 2023/3/6.
//

#ifndef PATH_TRACING_MESH_H
#define PATH_TRACING_MESH_H

#include "Triangle.h"
#include "../material/Material.h"
#include "../BVH.h"
#include <vector>

class Mesh {
public:
    string name = "A Mesh";
    std::vector<Triangle> triangles;    /**< triangles in local space >**/

    BVHNode *meshBVHRoot;
    Material* material;        /**< material resource. */

    void build_meshBVH();

    Mesh(string _name, std::vector<Triangle> &&_triangles);
    Mesh() = delete;
};


#endif //PATH_TRACING_MESH_H
