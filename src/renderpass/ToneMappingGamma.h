//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TONEMAPPINGGAMMA_H
#define PATH_TRACING_TONEMAPPINGGAMMA_H

#include "RenderPass.h"
#include "GBuffer.h"

class ToneMappingGamma: public VertexFragmentRenderPass {
public:

    ToneMappingGamma(const string &fragShaderPath);

    void draw(GBuffer &curFrame);
};


#endif //PATH_TRACING_TONEMAPPINGGAMMA_H
