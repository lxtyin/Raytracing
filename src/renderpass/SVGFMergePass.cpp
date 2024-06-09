//
// Created by 19450 on 2024/4/22.
//

#include "SVGFMergePass.h"

SVGFMergePass::SVGFMergePass(const string &fragShaderPath) :
VertexFragmentRenderPass(fragShaderPath),
colorGBufferSSBO(SCREEN_H * SCREEN_W * 3)
{}

SVGFMergePass::~SVGFMergePass() {
    colorGBufferSSBO.release();
}

void SVGFMergePass::draw(const SSBOBuffer<float> &directLumGBufferSSBO,
                         const SSBOBuffer<float> &indirectLumGBufferSSBO,
                         const SSBOBuffer<float> &albedoGBufferSSBO) {

    colorGBufferSSBO.bind_current_shader(0);
    directLumGBufferSSBO.bind_current_shader(1);
    indirectLumGBufferSSBO.bind_current_shader(2);
    albedoGBufferSSBO.bind_current_shader(3);

    drawcall();
}


