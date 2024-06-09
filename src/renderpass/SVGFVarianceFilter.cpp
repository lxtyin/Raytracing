//
// Created by 19450 on 2024/4/24.
//

#include "SVGFVarianceFilter.h"

SVGFVarianceFilter::SVGFVarianceFilter(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    varianceGBufferSSBO(SCREEN_W * SCREEN_H * 1)
{}

SVGFVarianceFilter::~SVGFVarianceFilter() {
    varianceGBufferSSBO.release();
}

void SVGFVarianceFilter::draw(const SSBOBuffer<float> &momentGBufferSSBO,
                              const SSBOBuffer<float> &normalGBufferSSBO,
                              const SSBOBuffer<float> &depthGBufferSSBO,
                              const SSBOBuffer<float> &numSamplesGBufferSSBO) {

    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_W"), SCREEN_W);
    glUniform1i(glGetUniformLocation(shaderProgram, "SCREEN_H"), SCREEN_H);

    momentGBufferSSBO.bind_current_shader(0);
    normalGBufferSSBO.bind_current_shader(1);
    depthGBufferSSBO.bind_current_shader(2);
    numSamplesGBufferSSBO.bind_current_shader(3);
    varianceGBufferSSBO.bind_current_shader(4);

    drawcall();
}
