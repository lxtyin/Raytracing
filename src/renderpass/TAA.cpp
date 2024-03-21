//
// Created by 19450 on 2024/3/17.
//

#include "TAA.h"
#include "../Config.h"

TAA::TAA(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {
    firstFrame = true;
}


void TAA::draw(GBuffer &curFrame, bool saveFrame) {

    if(firstFrame) {
        firstFrame = false;
        history.copyFrom(&curFrame);
        return;
    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO); // inout
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, history.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, curFrame.motionGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, curFrame.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, curFrame.meshIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, history.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, history.meshIndexGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    if(saveFrame) history.copyFrom(&curFrame);
    else history.swap(&curFrame);
}
