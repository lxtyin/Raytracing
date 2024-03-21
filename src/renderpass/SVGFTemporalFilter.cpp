//
// Created by 19450 on 2024/3/18.
//

#include "SVGFTemporalFilter.h"
#include "../Config.h"
#include "../tool/tool.h"

SVGFTemporalFilter::SVGFTemporalFilter(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}

void SVGFTemporalFilter::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, curFrame.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, curFrame.instanceIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, curFrame.motionGBufferSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, history.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, history.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, history.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, history.instanceIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, history.numSamplesGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, curFrame.numSamplesGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    history.copyFrom(&curFrame);
}