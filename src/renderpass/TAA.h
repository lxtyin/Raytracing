//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TAA_H
#define PATH_TRACING_TAA_H

#include "RenderPass.h"

class TAA: public VertexFragmentRenderPass {
    SSBOBuffer<float> historycolorGBufferSSBO;
    SSBOBuffer<float> historynormalGBufferSSBO;
    SSBOBuffer<float> historyinstanceIndexGBufferSSBO;
public:
    // output
    SSBOBuffer<float> outputColorGBufferSSBO;

    bool firstFrame = true;

    TAA(const string &fragShaderPath);
    ~TAA();

    void draw(const SSBOBuffer<float> &colorGBufferSSBO,
              const SSBOBuffer<float> &motionGBufferSSBO,
              const SSBOBuffer<float> &normalGBufferSSBO,
              const SSBOBuffer<float> &instanceIndexGBufferSSBO);
};


#endif //PATH_TRACING_TAA_H
