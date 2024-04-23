//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TAA_H
#define PATH_TRACING_TAA_H

#include "RenderPass.h"

class TAA: public VertexFragmentRenderPass {
public:

    SSBOBuffer<float> historycolorGBufferSSBO;
    SSBOBuffer<float> historynormalGBufferSSBO;
    SSBOBuffer<float> historyinstanceIndexGBufferSSBO;

    bool firstFrame = true;

    TAA(const string &fragShaderPath);
    ~TAA();

    /**
     * Note curFrame will be stolen if saveFrame = false;
     * \saveFrame set false only if it is the final pass.
     */
    void draw(SSBOBuffer<float> &colorGBufferSSBO,
              SSBOBuffer<float> &motionGBufferSSBO,
              SSBOBuffer<float> &normalGBufferSSBO,
              SSBOBuffer<float> &instanceIndexGBufferSSBO, bool saveFrame = true);
};


#endif //PATH_TRACING_TAA_H
