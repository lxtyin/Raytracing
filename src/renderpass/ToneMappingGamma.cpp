//
// Created by 19450 on 2024/3/17.
//

#include "ToneMappingGamma.h"

ToneMappingGamma::ToneMappingGamma(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath),
    outputColorGBufferSSBO(SCREEN_W * SCREEN_H * 3){}

void ToneMappingGamma::draw(const SSBOBuffer<float> &colorGBufferSSBO) {
    colorGBufferSSBO.bind_current_shader(0);
    outputColorGBufferSSBO.bind_current_shader(1);
    drawcall();
}

ToneMappingGamma::~ToneMappingGamma() {
    outputColorGBufferSSBO.release();
}


