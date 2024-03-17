//
// Created by 19450 on 2024/3/17.
//

#include "ToneMappingGamma.h"

ToneMappingGamma::ToneMappingGamma(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}

void ToneMappingGamma::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}


