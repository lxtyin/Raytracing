//
// Created by 19450 on 2024/3/18.
//

#include "SVGFTemporalFilter.h"
#include "../Config.h"
#include "../tool/tool.h"

SVGFTemporalFilter::SVGFTemporalFilter(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {
    int framesize = SCREEN_H * SCREEN_W;
    float *placeholder = new float[framesize];

    glGenBuffers(1, &historyLengthSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, historyLengthSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glGenBuffers(1, &nextLengthSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nextLengthSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);
}

void SVGFTemporalFilter::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, curFrame.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, curFrame.meshIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, curFrame.motionGBufferSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, history.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, history.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, history.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, history.meshIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, historyLengthSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, nextLengthSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    std::swap(historyLengthSSBO, nextLengthSSBO);
    history.copyFrom(&curFrame);
}

SVGFTemporalFilter::~SVGFTemporalFilter() {
    glDeleteBuffers(1, &historyLengthSSBO);
    glDeleteBuffers(1, &nextLengthSSBO);
}
