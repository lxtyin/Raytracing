//
// Created by 19450 on 2024/4/22.
//

#ifndef PATH_TRACING_SVGFMERGEPASS_H
#define PATH_TRACING_SVGFMERGEPASS_H

#include "RenderPass.h"


class SVGFMergePass: public VertexFragmentRenderPass {
public:
    // Output
    SSBOBuffer<float> colorGBufferSSBO;

    SVGFMergePass(const string &fragShaderPath);
    ~SVGFMergePass();

    void draw(const SSBOBuffer<float> &directLumGBufferSSBO,
              const SSBOBuffer<float> &indirectLumGBufferSSBO,
              const SSBOBuffer<float> &albedoGBufferSSBO);
};


#endif //PATH_TRACING_SVGFMERGEPASS_H
