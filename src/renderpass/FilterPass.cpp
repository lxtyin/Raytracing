//
// Created by 19450 on 2024/3/17.
//

#include "FilterPass.h"

FilterPass::FilterPass(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}

void FilterPass::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, curFrame.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.depthGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}
