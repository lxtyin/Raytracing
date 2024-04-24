//
// Created by 19450 on 2024/4/24.
//

#ifndef PATH_TRACING_SVGFVARIANCEFILTER_H
#define PATH_TRACING_SVGFVARIANCEFILTER_H

#include "RenderPass.h"

class SVGFVarianceFilter: public VertexFragmentRenderPass {
public:
    // output
    SSBOBuffer<float> varianceGBufferSSBO;

    SVGFVarianceFilter(const string &fragShaderPath);
    ~SVGFVarianceFilter();

    void draw(const SSBOBuffer<float> &momentGBufferSSBO,
              const SSBOBuffer<float> &normalGBufferSSBO,
              const SSBOBuffer<float> &depthGBufferSSBO,
              const SSBOBuffer<float> &numSamplesGBufferSSBO);
};



#endif //PATH_TRACING_SVGFVARIANCEFILTER_H
