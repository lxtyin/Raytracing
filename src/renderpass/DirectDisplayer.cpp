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

    colorGBufferSSBO.bind_current_shader(0);
    instanceIndexGBufferSSBO.bind_current_shader(1);
    drawcall();
}


