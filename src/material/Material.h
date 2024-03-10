//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_MATERIAL_H
#define PATH_TRACING_MATERIAL_H

#include "../instance/Triangle.h"
#include "../texture/Texture.h"
#include <map>

class Material {
public:
    string name = "A Material";

    virtual void insert_gui() = 0;

    /**
     * append this material info into the buffer.
     * @param materialBuffer
     * @param textureIndexMap
     * @return ptr in the materialBuffer.
     */
    virtual int insert_buffer(std::vector<float> &materialBuffer, const std::map<Texture*, uint> &textureIndexMap) = 0;

    /**
     * returns all texture included in this material
     * @return
     */
    virtual std::vector<Texture*> textures() = 0;
};



#endif //PATH_TRACING_MATERIAL_H
