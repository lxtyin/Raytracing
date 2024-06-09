//
// Created by 19450 on 2024/3/18.
//

#ifndef PATH_TRACING_SVGFTEMPORALFILTER_H
#define PATH_TRACING_SVGFTEMPORALFILTER_H

#include "RenderPass.h"

class SVGFTemporalFilter: public VertexFragmentRenderPass {
    // stores
    SSBOBuffer<float> historyColorGBufferSSBO;
    SSBOBuffer<float> historyMomentGBufferSSBO;
    SSBOBuffer<float> historyNumSamplesGBufferSSBO;
    SSBOBuffer<float> historynormalGBufferSSBO;
    SSBOBuffer<float> historyinstanceIndexGBufferSSBO;
public:
    // outputs.
    SSBOBuffer<float> outputColorGBufferSSBO;
    SSBOBuffer<float> outputMomentGBufferSSBO;
    SSBOBuffer<float> outputNumSamplesGBufferSSBO;

    SVGFTemporalFilter(const string &fragShaderPath);
    ~SVGFTemporalFilter();

    bool firstFrame = true;

    void update_historycolor(const SSBOBuffer<float> &colorGBufferSSBO);

    void draw(const SSBOBuffer<float> &colorGBufferSSBO,
              const SSBOBuffer<float> &normalGBufferSSBO,
              const SSBOBuffer<float> &instanceIndexGBufferSSBO,
              const SSBOBuffer<float> &motionGBufferSSBO);
};


#endif //PATH_TRACING_SVGFTEMPORALFILTER_H
