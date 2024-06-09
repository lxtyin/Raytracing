//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TONEMAPPINGGAMMA_H
#define PATH_TRACING_TONEMAPPINGGAMMA_H

#include "RenderPass.h"

class ToneMappingGamma: public VertexFragmentRenderPass {
public:
    SSBOBuffer<float> outputColorGBufferSSBO;

    ToneMappingGamma(const string &fragShaderPath);
    ~ToneMappingGamma();

    void draw(const SSBOBuffer<float> &colorGBufferSSBO);
};


#endif //PATH_TRACING_TONEMAPPINGGAMMA_H
