//
// Created by 19450 on 2024/4/17.
//

#ifndef PATH_TRACING_STATICBLENDER_H
#define PATH_TRACING_STATICBLENDER_H

#include "RenderPass.h"

class StaticBlender: public VertexFragmentRenderPass {
public:
    SSBOBuffer<float> historyColorGBufferSSBO;

    int frameCounter;

    StaticBlender(const string &fragShaderPath);
    ~StaticBlender();

    /**
     * Note curFrame will be stolen if saveFrame = false;
     * \saveFrame set false only if it is the final pass.
     */
    void draw(SSBOBuffer<float> &colorGBufferSSBO);
};



#endif //PATH_TRACING_STATICBLENDER_H
