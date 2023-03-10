//
// Created by lx_tyin on 2022/11/24.
//

#ifndef OPENGL_INSTANCE_H
#define OPENGL_INSTANCE_H

#include "Mesh.h"
#include "../Transform.h"
#include <vector>
using std::vector;

class Instance {

    Instance *parent = nullptr;
    vector<Instance*> children;
public:
    vector<Mesh*> meshes;
    Transform transform;                 /**< transform to parent. */

    Instance() = default;
    Instance(Instance *p);

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
