//
// Created by 19450 on 2024/3/17.
//

#ifndef PATH_TRACING_TONEMAPMIX_H
#define PATH_TRACING_TONEMAPMIX_H

#include "RenderPass.h"
#include "GBuffer.h"

class ToneMapMix: public VertexFragmentRenderPass {
public:
    GLuint outputSSBO;

    ToneMapMix(const string &fragShaderPath);

    void draw(GBuffer &curFrame, GBuffer &lastFrame);
};


#endif //PATH_TRACING_TONEMAPMIX_H
