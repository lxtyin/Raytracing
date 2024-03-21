//
// Created by 19450 on 2024/3/15.
//

#include "glad/glad.h"
#include "DirectDisplayer.h"
#include "../Config.h"
#include <iostream>

DirectDisplayer::DirectDisplayer(const string &fragShaderPath) : VertexFragmentRenderPass(fragShaderPath) {}


void DirectDisplayer::draw(GBuffer &curFrame) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, curFrame.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, curFrame.instanceIndexGBufferSSBO);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}


