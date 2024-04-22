//
// Created by 19450 on 2024/4/17.
//

#include "StaticBlender.h"
#include "../tool/tool.h"
#include "../Config.h"

StaticBlender::StaticBlender(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    historyColorGBufferSSBO(SCREEN_W * SCREEN_H * 3)
{
    frameCounter = 0;
}

void StaticBlender::draw(SSBOBuffer<float> &colorGBufferSSBO) {
    ++frameCounter;

    glUniform1ui(glGetUniformLocation(shaderProgram, "frameCounter"), frameCounter);

    colorGBufferSSBO.bind_current_shader(0);
    historyColorGBufferSSBO.bind_current_shader(1);

    drawcall();
}

StaticBlender::~StaticBlender() {
    historyColorGBufferSSBO.release();
}

