//
// Created by 19450 on 2024/3/17.
//

#include "ToneMappingGamma.h"

ToneMappingGamma::ToneMappingGamma(const string &fragShaderPath) :
    VertexFragmentRenderPass(fragShaderPath){}

void ToneMappingGamma::draw(SSBOBuffer<float> &colorGBufferSSBO) {
    colorGBufferSSBO.bind_current_shader(0);
    drawcall();
}


