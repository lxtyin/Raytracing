//
// Created by 19450 on 2024/3/17.
//

#include "SVGFSpatialFilterPass.h"
#include "../tool/tool.h"
#include "../Config.h"

SVGFSpatialFilterPass::SVGFSpatialFilterPass(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    colorOutputSSBO(SCREEN_H * SCREEN_W * 3) {}

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
    colorOutputSSBO.bind_current_shader(5);

    drawcall();
    colorOutputSSBO.copy(&colorGBufferSSBO);
}

SVGFSpatialFilterPass::~SVGFSpatialFilterPass() {
    colorOutputSSBO.release();
}
