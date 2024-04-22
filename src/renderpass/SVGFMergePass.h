//
// Created by 19450 on 2024/4/22.
//

#ifndef PATH_TRACING_SVGFMERGEPASS_H
#define PATH_TRACING_SVGFMERGEPASS_H

#include "RenderPass.h"


class SVGFMergePass: public VertexFragmentRenderPass {
public:
    SSBOBuffer<float> colorGBufferSSBO;

    SVGFMergePass(const string &fragShaderPath);
    ~SVGFMergePass();

    void draw(SSBOBuffer<float> &directLumGBufferSSBO,
              SSBOBuffer<float> &indirectLumGBufferSSBO,
              SSBOBuffer<float> &albedoGBufferSSBO);
};


#endif //PATH_TRACING_SVGFMERGEPASS_H
