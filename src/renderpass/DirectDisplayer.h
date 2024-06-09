//
// Created by 19450 on 2024/3/15.
//

#ifndef PATH_TRACING_DIRECTDISPLAYER_H
#define PATH_TRACING_DIRECTDISPLAYER_H

#include "RenderPass.h"

// This pass will display the texture on screen directly.
class DirectDisplayer: public VertexFragmentRenderPass {
public:
    DirectDisplayer(const string &fragShaderPath);

    void draw(const SSBOBuffer<float> &renderedGBufferSSBO,
              const SSBOBuffer<float> &directLumGBufferSSBO,
              const SSBOBuffer<float> &indirectLumGBuffer,
              const SSBOBuffer<float> &albedoGBufferSSBO,
              const SSBOBuffer<float> &depthGBufferSSBO,
              const SSBOBuffer<float> &normalGBufferSSBO,
              const SSBOBuffer<float> &instanceIndexGBufferSSBO);
};


#endif //PATH_TRACING_DIRECTDISPLAYER_H
