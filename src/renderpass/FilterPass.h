//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_FILTERPASS_H
#define PATH_TRACING_FILTERPASS_H

#include "RenderPass.h"

class FilterPass: public VertexFragmentRenderPass {
public:
    FilterPass(const string &fragShaderPath);

    void draw(GBuffer &curFrame);
};


#endif //PATH_TRACING_FILTERPASS_H
