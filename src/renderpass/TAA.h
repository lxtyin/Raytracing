//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TAA_H
#define PATH_TRACING_TAA_H

#include "RenderPass.h"
#include "GBuffer.h"


class TAA: public VertexFragmentRenderPass {
public:
    TAA(const string &fragShaderPath);

    void draw(GBuffer &curFrame, GBuffer &lastFrame);
};


#endif //PATH_TRACING_TAA_H
