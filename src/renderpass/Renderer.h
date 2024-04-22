//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "../instance/Triangle.h"
#include "RenderPass.h"
#include <vector>
#include <map>

class Scene;
class Instance;

class Renderer: public VertexFragmentRenderPass {
public:
    SSBOBuffer<float> colorGBufferSSBO;
    SSBOBuffer<float> motionGBufferSSBO;
    SSBOBuffer<float> albedoGBufferSSBO;
    SSBOBuffer<float> momentGBufferSSBO;
    SSBOBuffer<float> numSamplesGBufferSSBO;
    SSBOBuffer<float> depthGBufferSSBO;
    SSBOBuffer<float> normalGBufferSSBO;
    SSBOBuffer<float> instanceIndexGBufferSSBO;

    Renderer(const string &shaderPath);
    ~Renderer();

    void draw();
};


#endif //PATH_TRACING_RENDERER_H
