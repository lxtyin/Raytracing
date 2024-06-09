//
// Created by 19450 on 2024/3/17.
//

#include "TAA.h"
#include "../Config.h"

TAA::TAA(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    historycolorGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historynormalGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historyinstanceIndexGBufferSSBO(SCREEN_H * SCREEN_W * 1),
    outputColorGBufferSSBO(SCREEN_H * SCREEN_W * 3)
    { firstFrame = true;}

void TAA::draw(const SSBOBuffer<float> &colorGBufferSSBO,
               const SSBOBuffer<float> &motionGBufferSSBO,
               const SSBOBuffer<float> &normalGBufferSSBO,
               const SSBOBuffer<float> &instanceIndexGBufferSSBO) {

    if(firstFrame) {
        firstFrame = false;
        historycolorGBufferSSBO.copy(&colorGBufferSSBO);
        historynormalGBufferSSBO.copy(&normalGBufferSSBO);
        historyinstanceIndexGBufferSSBO.copy(&instanceIndexGBufferSSBO);
        return;
    }

    colorGBufferSSBO.bind_current_shader(0);
    historycolorGBufferSSBO.bind_current_shader(1);
    motionGBufferSSBO.bind_current_shader(2);
    normalGBufferSSBO.bind_current_shader(3);
    instanceIndexGBufferSSBO.bind_current_shader(4);
    historynormalGBufferSSBO.bind_current_shader(5);
    historyinstanceIndexGBufferSSBO.bind_current_shader(6);
    outputColorGBufferSSBO.bind_current_shader(7);

    drawcall();

    historycolorGBufferSSBO.copy(&outputColorGBufferSSBO);
    historynormalGBufferSSBO.copy(&normalGBufferSSBO);
    historyinstanceIndexGBufferSSBO.copy(&instanceIndexGBufferSSBO);
}

TAA::~TAA() {
    historycolorGBufferSSBO.release();
    historynormalGBufferSSBO.release();
    historyinstanceIndexGBufferSSBO.release();
    outputColorGBufferSSBO.release();
}
