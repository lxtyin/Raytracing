//
// Created by lx_tyin on 2023/3/6.
//

#include "Mesh.h"

void Mesh::build_meshBVH() {

    std::vector<BVHPrimitive> primitives;
    for(Triangle &t: triangles) {
        BVHPrimitive tmp;
        assert(!tmp.trianglePtr);
        assert(!tmp.meshPtr);

        tmp.trianglePtr = &t;
        for(auto &v: t.vertex) tmp.aabb.addPoint(v);
        primitives.push_back(tmp);
    }

    meshBVHRoot = BVHNode::build(primitives);
    meshBVHRoot->meshPtr = this;
}

Mesh::Mesh(string _name, std::vector<Triangle> &&_triangles):
    name(_name), triangles(_triangles)
{
    assert(_triangles.size() > 0);
    build_meshBVH();
}

Mesh::~Mesh() {
    delete meshBVHRoot;
}
