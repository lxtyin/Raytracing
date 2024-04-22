//
// Created by 19450 on 2024/3/18.
//

#ifndef PATH_TRACING_SVGFTEMPORALFILTER_H
#define PATH_TRACING_SVGFTEMPORALFILTER_H

#include "RenderPass.h"

class SVGFTemporalFilter: public VertexFragmentRenderPass {
public:

    SSBOBuffer<float> historycolorGBufferSSBO;
    SSBOBuffer<float> historymomentGBufferSSBO;
    SSBOBuffer<float> historynormalGBufferSSBO;
    SSBOBuffer<float> historyinstanceIndexGBufferSSBO;
    SSBOBuffer<float> historynumSamplesGBufferSSBO;

    SVGFTemporalFilter(const string &fragShaderPath);
    ~SVGFTemporalFilter();

    bool firstFrame = true;

    void draw(SSBOBuffer<float> &colorGBufferSSBO,
              SSBOBuffer<float> &momentGBufferSSBO,
              SSBOBuffer<float> &normalGBufferSSBO,
              SSBOBuffer<float> &instanceIndexGBufferSSBO,
              SSBOBuffer<float> &motionGBufferSSBO,
              SSBOBuffer<float> &numSamplesGBufferSSBO);
};


#endif //PATH_TRACING_SVGFTEMPORALFILTER_H
