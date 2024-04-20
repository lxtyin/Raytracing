//
// Created by 19450 on 2024/3/18.
//

#include "SVGFTemporalFilter.h"
#include "../Config.h"
#include "../tool/tool.h"

SVGFTemporalFilter::SVGFTemporalFilter(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    historycolorGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historymomentGBufferSSBO(SCREEN_H * SCREEN_W * 2),
    historynormalGBufferSSBO(SCREEN_H * SCREEN_W * 3),
    historyinstanceIndexGBufferSSBO(SCREEN_H * SCREEN_W * 1),
    historynumSamplesGBufferSSBO(SCREEN_H * SCREEN_W * 1)
    {}

void SVGFTemporalFilter::draw(SSBOBuffer<float> &colorGBufferSSBO,
                              SSBOBuffer<float> &momentGBufferSSBO,
                              SSBOBuffer<float> &normalGBufferSSBO,
                              SSBOBuffer<float> &instanceIndexGBufferSSBO,
                              SSBOBuffer<float> &motionGBufferSSBO,
                              SSBOBuffer<float> &numSamplesGBufferSSBO) {

    colorGBufferSSBO.bind_current_shader(0);
    momentGBufferSSBO.bind_current_shader(1);
    normalGBufferSSBO.bind_current_shader(2);
    instanceIndexGBufferSSBO.bind_current_shader(3);
    motionGBufferSSBO.bind_current_shader(4);

    historycolorGBufferSSBO.bind_current_shader(5);
    historymomentGBufferSSBO.bind_current_shader(6);
    historynormalGBufferSSBO.bind_current_shader(7);
    historyinstanceIndexGBufferSSBO.bind_current_shader(8);
    historynumSamplesGBufferSSBO.bind_current_shader(9);

    numSamplesGBufferSSBO.bind_current_shader(10);

    drawcall();

    historycolorGBufferSSBO.copy(&colorGBufferSSBO);
    historycolorGBufferSSBO.copy(&colorGBufferSSBO);
    historymomentGBufferSSBO.copy(&momentGBufferSSBO);
    historynormalGBufferSSBO.copy(&normalGBufferSSBO);
    historyinstanceIndexGBufferSSBO.copy(&instanceIndexGBufferSSBO);
    historynumSamplesGBufferSSBO.copy(&numSamplesGBufferSSBO);
}

SVGFTemporalFilter::~SVGFTemporalFilter() {
    historycolorGBufferSSBO.release();
    historymomentGBufferSSBO.release();
    historynormalGBufferSSBO.release();
    historyinstanceIndexGBufferSSBO.release();
    historynumSamplesGBufferSSBO.release();
}