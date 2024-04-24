//
// Created by 19450 on 2024/4/17.
//

#ifndef PATH_TRACING_STATICBLENDER_H
#define PATH_TRACING_STATICBLENDER_H

#include "RenderPass.h"

class StaticBlender: public VertexFragmentRenderPass {
public:
    // Outputs
    SSBOBuffer<float> historyColorGBufferSSBO;
    SSBOBuffer<float> historyMomentGBufferSSBO;

    int frameCounter;

    StaticBlender(const string &fragShaderPath);
    ~StaticBlender();

    void draw(const SSBOBuffer<float> &colorGBufferSSBO);
};



#endif //PATH_TRACING_STATICBLENDER_H
