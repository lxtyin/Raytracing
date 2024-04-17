//
// Created by 19450 on 2024/4/17.
//

#include "StaticBlender.h"
#include "../tool/tool.h"
#include "../Config.h"

StaticBlender::StaticBlender(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {
    frameCounter = 0;

    int framesize = SCREEN_H * SCREEN_W * 3;
    float *placeholder = new float[framesize];

    glGenBuffers(1, &historyColorGBufferSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, historyColorGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);

}

void StaticBlender::draw(GBuffer &curFrame) {
    ++frameCounter;

    glUniform1ui(glGetUniformLocation(shaderProgram, "frameCounter"), frameCounter);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, historyColorGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

StaticBlender::~StaticBlender() {
    glDeleteBuffers(1, &historyColorGBufferSSBO);
}
