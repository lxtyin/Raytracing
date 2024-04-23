//
// Created by 19450 on 2024/3/15.
//

#include "glad/glad.h"
#include "DirectDisplayer.h"
#include "../Config.h"
#include <iostream>

DirectDisplayer::DirectDisplayer(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}


void DirectDisplayer::draw(SSBOBuffer<float> &colorGBufferSSBO,
                           SSBOBuffer<float> &instanceIndexGBufferSSBO) {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    colorGBufferSSBO.bind_current_shader(0);
    instanceIndexGBufferSSBO.bind_current_shader(1);
    drawcall();
}


