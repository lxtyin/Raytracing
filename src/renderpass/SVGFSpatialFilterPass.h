//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_SVGFSPATIALFILTERPASS_H
#define PATH_TRACING_SVGFSPATIALFILTERPASS_H

#include "RenderPass.h"

class SVGFSpatialFilterPass: public VertexFragmentRenderPass {
public:

    SSBOBuffer<float> colorOutputGBufferSSBO;

    SVGFSpatialFilterPass(const string &fragShaderPath);
    ~SVGFSpatialFilterPass();


    void draw(SSBOBuffer<float> &colorGBufferSSBO,
              SSBOBuffer<float> &normalGBufferSSBO,
              SSBOBuffer<float> &depthGBufferSSBO,
              SSBOBuffer<float> &momentGBufferSSBO,
              SSBOBuffer<float> &numSamplesGBufferSSBO);
};


#endif //PATH_TRACING_SVGFSPATIALFILTERPASS_H
