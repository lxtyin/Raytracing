//
// Created by lx_tyin on 2023/3/6.
//

#ifndef PATH_TRACING_MESH_H
#define PATH_TRACING_MESH_H

#include "Triangle.h"
#include "Material.h"
#include <vector>
using std::vector;

class Mesh {
public:
    string name = "A Mesh";
    vector<Triangle> triangles;    /**< triangles in local space >**/
    Material* material;        /**< material resource. */
};


#endif //PATH_TRACING_MESH_H
