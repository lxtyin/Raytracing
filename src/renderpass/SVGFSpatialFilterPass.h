//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_SVGFSPATIALFILTERPASS_H
#define PATH_TRACING_SVGFSPATIALFILTERPASS_H

#include "RenderPass.h"

class SVGFSpatialFilterPass: public VertexFragmentRenderPass {
    SSBOBuffer<float> tmpColorGBufferSSBO;
public:
    // output
    SSBOBuffer<float> outputColorGBufferSSBO;

    SVGFSpatialFilterPass(const string &fragShaderPath);
    ~SVGFSpatialFilterPass();

    void draw(const SSBOBuffer<float> &colorGBufferSSBO,
              const SSBOBuffer<float> &varianceGBufferSSBO,
              const SSBOBuffer<float> &normalGBufferSSBO,
              const SSBOBuffer<float> &depthGBufferSSBO);
};


#endif //PATH_TRACING_SVGFSPATIALFILTERPASS_H
