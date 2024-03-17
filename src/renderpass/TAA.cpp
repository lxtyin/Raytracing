//
// Created by 19450 on 2024/3/17.
//

#include "TAA.h"

TAA::TAA(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) { }

void TAA::draw(GBuffer &curFrame, GBuffer &lastFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lastFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.motionGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}
