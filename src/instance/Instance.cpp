//
// Created by lx_tyin on 2022/11/24.
//

#include "Instance.h"

Instance::Instance(const string &nm, Instance *p): name(nm), parent(p) {
    if(p) p->children.push_back(p);
}

Instance *Instance::get_parent() {
    return parent;
}

Instance *Instance::get_child(int idx) {
    if(idx >= children.size()) return nullptr;
    return children[idx];
}

void Instance::set_parent(Instance *p) {
    if(parent != nullptr){
        for(auto it = parent->children.begin();it != parent->children.end();it++){
            if(*it == this){
                parent->children.erase(it);
                break;
            }
        }
    }
    parent = p;
    if(p) p->children.push_back(this);
}

int Instance::add_child(Instance *cd) {
    cd->set_parent(this);
    return (int)children.size() - 1;
}

mat4 Instance::matrix_to_global() {
	if(parent == nullptr) return transform.matrix();
	return parent->matrix_to_global() * transform.matrix();
}

Instance::~Instance() {
    for(auto it: children) {
        it->parent = nullptr;
        delete it;
    }
    if(parent) set_parent(nullptr);
}
