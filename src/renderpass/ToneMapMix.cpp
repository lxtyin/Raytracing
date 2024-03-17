//
// Created by 19450 on 2024/3/17.
//

#include "ToneMapMix.h"
#include "../Config.h"

ToneMapMix::ToneMapMix(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}


void ToneMapMix::draw(GBuffer &curFrame, GBuffer &lastFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lastFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.motionGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}


