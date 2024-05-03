//
// Created by 19450 on 2024/3/15.
//

#include "glad/glad.h"
#include "DirectDisplayer.h"
#include "../Config.h"
#include <iostream>

DirectDisplayer::DirectDisplayer(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}


void DirectDisplayer::draw(
        const SSBOBuffer<float> &renderedGBufferSSBO,
        const SSBOBuffer<float> &directLumGBufferSSBO,
        const SSBOBuffer<float> &indirectLumGBuffer,
        const SSBOBuffer<float> &albedoGBufferSSBO,
        const SSBOBuffer<float> &depthGBufferSSBO,
        const SSBOBuffer<float> &normalGBufferSSBO,
        const SSBOBuffer<float> &instanceIndexGBufferSSBO) {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderedGBufferSSBO.bind_current_shader(0);
    directLumGBufferSSBO.bind_current_shader(1);
    indirectLumGBuffer.bind_current_shader(2);
    albedoGBufferSSBO.bind_current_shader(3);
    depthGBufferSSBO.bind_current_shader(4);
    normalGBufferSSBO.bind_current_shader(5);
    instanceIndexGBufferSSBO.bind_current_shader(6);
    drawcall();
}


