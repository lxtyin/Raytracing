//
// Created by lx_tyin on 2022/11/24.
//

#ifndef OPENGL_INSTANCE_H
#define OPENGL_INSTANCE_H

#include "../Transform.h"
#include <vector>

class Mesh;
class Material;

enum EmitterType {
    Emitter_NONE,
    Emitter_SURFACE,
    Emitter_POINT,
    Emitter_DIRECTIONAL
};

class Instance {
protected:
    friend class TinyUI;

    Instance *parent = nullptr;
    std::vector<Instance*> children;

public:
    string name = "A Instance";
    Mesh* mesh = nullptr;
    Material *material = nullptr;
    Transform transform;                 /**< transform to parent. */
    EmitterType emitterType = Emitter_NONE;
    vec3 emission = vec3(1.0);

    Instance() = default;
    ~Instance();

    explicit Instance(const string &nm, Instance *p = nullptr);

	mat4 matrix_to_global(); // O(Hierarchy depth), not large

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
