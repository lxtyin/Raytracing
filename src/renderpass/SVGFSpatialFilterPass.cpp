//
// Created by 19450 on 2024/3/17.
//

#include "SVGFSpatialFilterPass.h"
#include "../tool/tool.h"
#include "../Config.h"

SVGFSpatialFilterPass::SVGFSpatialFilterPass(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    colorOutputGBufferSSBO(SCREEN_H * SCREEN_W * 3) {}

void SVGFSpatialFilterPass::draw(SSBOBuffer<float> &colorGBufferSSBO,
                                 SSBOBuffer<float> &normalGBufferSSBO,
                                 SSBOBuffer<float> &depthGBufferSSBO,
                                 SSBOBuffer<float> &momentGBufferSSBO,
                                 SSBOBuffer<float> &numSamplesGBufferSSBO) {

    colorGBufferSSBO.bind_current_shader(0);
    normalGBufferSSBO.bind_current_shader(1);
    depthGBufferSSBO.bind_current_shader(2);
    momentGBufferSSBO.bind_current_shader(3);
    numSamplesGBufferSSBO.bind_current_shader(4);
    colorOutputGBufferSSBO.bind_current_shader(5);

    drawcall();
    colorGBufferSSBO.copy(&colorOutputGBufferSSBO);
}

SVGFSpatialFilterPass::~SVGFSpatialFilterPass() {
    colorOutputGBufferSSBO.release();
}
