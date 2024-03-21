//
// Created by 19450 on 2024/3/17.
//

#include "FilterPass.h"
#include "../tool/tool.h"
#include "../Config.h"

FilterPass::FilterPass(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {
    int framesize = SCREEN_H * SCREEN_W * 3;
    float *placeholder = new float[framesize];

    glGenBuffers(1, &colorOutputSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorOutputSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);
}

void FilterPass::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, curFrame.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.depthGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, curFrame.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, curFrame.numSamplesGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, colorOutputSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    copySSBO(colorOutputSSBO, curFrame.colorGBufferSSBO, SCREEN_H * SCREEN_W * 3 * sizeof(float));
}

FilterPass::~FilterPass() {
    glDeleteBuffers(1, &colorOutputSSBO);
}
