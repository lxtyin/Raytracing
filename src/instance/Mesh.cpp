//
// Created by lx_tyin on 2023/3/6.
//

#include "Mesh.h"
#include "../BVH.h"

void Mesh::build_meshBVH() {

    std::vector<BVHPrimitive> primitives;
    for(Triangle &t: triangles) {
        BVHPrimitive tmp;

        tmp.trianglePtr = &t;
        for(auto &v: t.vertex) tmp.aabb.addPoint(v);
        primitives.push_back(tmp);
    }

    meshBVHRoot = BVHNode::build(primitives);
}

Mesh::Mesh(string _name, std::vector<Triangle> &&_triangles):
    name(_name), triangles(_triangles)
{
    assert(_triangles.size() > 0);

    bind_vaovbo();
    build_meshBVH();
    cout << "Mesh: " << name << " BVH size: " << meshBVHRoot->siz << std::endl;
}

Mesh::~Mesh() {
    delete meshBVHRoot;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Mesh::bind_vaovbo() {
    int siz = triangles.size() * 3 * (3 + 3 + 2);
    float *tmpdata = new float[siz];
    int n = 0;
    for(Triangle &t: triangles) {
        for(int i = 0;i < 3;i++) {
            tmpdata[n++] = t.vertex[i].x;
            tmpdata[n++] = t.vertex[i].y;
            tmpdata[n++] = t.vertex[i].z;
            tmpdata[n++] = t.normal[i].x;
            tmpdata[n++] = t.normal[i].y;
            tmpdata[n++] = t.normal[i].z;
            tmpdata[n++] = t.uv[i].x;
            tmpdata[n++] = t.uv[i].y;
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, siz * sizeof(float), tmpdata, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(0 * sizeof(float)));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);//unbind VAO

    delete[] tmpdata;
}

void Mesh::draw_in_rasterization() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, triangles.size() * 3);
    glBindVertexArray(0);
}
