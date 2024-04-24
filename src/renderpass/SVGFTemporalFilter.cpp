//
// Created by 19450 on 2024/3/18.
//

#include "SVGFTemporalFilter.h"
#include "../Config.h"
#include "../tool/tool.h"

SVGFTemporalFilter::SVGFTemporalFilter(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    outputColorGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    outputMomentGBufferSSBO(SCREEN_H * SCREEN_W * 2),
    outputNumSamplesGBufferSSBO(SCREEN_H * SCREEN_W * 1),
    historyColorGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historyMomentGBufferSSBO(SCREEN_H * SCREEN_W * 2),
    historyNumSamplesGBufferSSBO(SCREEN_H * SCREEN_W * 1),
    historynormalGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historyinstanceIndexGBufferSSBO(SCREEN_H * SCREEN_W * 1)
    {}

void SVGFTemporalFilter::draw(const SSBOBuffer<float> &colorGBufferSSBO,
                              const SSBOBuffer<float> &normalGBufferSSBO,
                              const SSBOBuffer<float> &instanceIndexGBufferSSBO,
                              const SSBOBuffer<float> &motionGBufferSSBO) {

    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_W"), SCREEN_W);
    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_H"), SCREEN_H);

    if(firstFrame) firstFrame = false;
    glUniform1i(glGetUniformLocation(shaderProgram, "firstFrame"), firstFrame);

    colorGBufferSSBO.bind_current_shader(0);
    normalGBufferSSBO.bind_current_shader(1);
    instanceIndexGBufferSSBO.bind_current_shader(2);
    motionGBufferSSBO.bind_current_shader(3);

    outputColorGBufferSSBO.bind_current_shader(4);
    outputMomentGBufferSSBO.bind_current_shader(5);
    outputNumSamplesGBufferSSBO.bind_current_shader(6);

    historyColorGBufferSSBO.bind_current_shader(7);
    historyMomentGBufferSSBO.bind_current_shader(8);
    historyNumSamplesGBufferSSBO.bind_current_shader(9);
    historynormalGBufferSSBO.bind_current_shader(10);
    historyinstanceIndexGBufferSSBO.bind_current_shader(11);

    drawcall();

    historyColorGBufferSSBO.copy(&outputColorGBufferSSBO);
    historyMomentGBufferSSBO.copy(&outputMomentGBufferSSBO);
    historyNumSamplesGBufferSSBO.copy(&outputNumSamplesGBufferSSBO);
    historynormalGBufferSSBO.copy(&normalGBufferSSBO);
    historyinstanceIndexGBufferSSBO.copy(&instanceIndexGBufferSSBO);
}

SVGFTemporalFilter::~SVGFTemporalFilter() {
    outputColorGBufferSSBO.release();
    outputMomentGBufferSSBO.release();
    outputNumSamplesGBufferSSBO.release();
    historyColorGBufferSSBO.release();
    historyMomentGBufferSSBO.release();
    historyNumSamplesGBufferSSBO.release();
    historynormalGBufferSSBO.release();
    historyinstanceIndexGBufferSSBO.release();
}