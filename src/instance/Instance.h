//
// Created by lx_tyin on 2022/11/24.
//

#ifndef OPENGL_INSTANCE_H
#define OPENGL_INSTANCE_H

#include "Mesh.h"
#include "../Transform.h"
#include <vector>

class Instance {
protected:

    friend class TinyUI;

    Instance *parent = nullptr;
    std::vector<Instance*> children;

public:
    std::vector<Mesh*> meshes;
    Mesh* mesh = nullptr;
    string name = "A Instance";
    Transform transform;                 /**< transform to parent. */

    Instance() = default;
    explicit Instance(const string &nm, Instance *p = nullptr);

	mat4 matrix_to_global();

    Instance* get_parent();

    Instance* get_child(int idx);

    void set_parent(Instance *p);

    /**
     * add instacne as a child, meanwhile update it's parent.
     * \param cd child instance
     * \return index of this child.
     */
    int add_child(Instance *cd);
};

#endif //OPENGL_INSTANCE_H
