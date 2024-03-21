//
// Created by 19450 on 2024/3/17.
//

#include "GBuffer.h"
#include "Config.h"
#include "tool/tool.h"

GBuffer::GBuffer() {
    glGenBuffers(1, &colorGBufferSSBO);
    glGenBuffers(1, &normalGBufferSSBO);
    glGenBuffers(1, &depthGBufferSSBO);
    glGenBuffers(1, &motionGBufferSSBO);
    glGenBuffers(1, &albedoGBufferSSBO);
    glGenBuffers(1, &momentGBufferSSBO);
    glGenBuffers(1, &meshIndexGBufferSSBO);
    glGenBuffers(1, &numSamplesGBufferSSBO);

    int framesize = SCREEN_H * SCREEN_W;
    float *placeholder = new float[framesize * 3];

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 3 * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, normalGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 3 * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, depthGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, motionGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 2 * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, albedoGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 3 * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, momentGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 2 * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, meshIndexGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, numSamplesGBufferSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * sizeof(float), placeholder, GL_DYNAMIC_COPY);

    delete[] placeholder;
}

GBuffer::~GBuffer() {
    glDeleteBuffers(1, &colorGBufferSSBO);
    glDeleteBuffers(1, &normalGBufferSSBO);
    glDeleteBuffers(1, &depthGBufferSSBO);
    glDeleteBuffers(1, &motionGBufferSSBO);
    glDeleteBuffers(1, &albedoGBufferSSBO);
    glDeleteBuffers(1, &momentGBufferSSBO);
    glDeleteBuffers(1, &meshIndexGBufferSSBO);
    glDeleteBuffers(1, &numSamplesGBufferSSBO);
}

void GBuffer::swap(GBuffer *buffer) {
    std::swap(colorGBufferSSBO, buffer->colorGBufferSSBO);
    std::swap(normalGBufferSSBO, buffer->normalGBufferSSBO);
    std::swap(depthGBufferSSBO, buffer->depthGBufferSSBO);
    std::swap(motionGBufferSSBO, buffer->motionGBufferSSBO);
    std::swap(albedoGBufferSSBO, buffer->albedoGBufferSSBO);
    std::swap(momentGBufferSSBO, buffer->momentGBufferSSBO);
    std::swap(meshIndexGBufferSSBO, buffer->meshIndexGBufferSSBO);
    std::swap(numSamplesGBufferSSBO, buffer->numSamplesGBufferSSBO);
}

void GBuffer::copyFrom(GBuffer *buffer) {
    int framesize = SCREEN_W * SCREEN_H * sizeof(float);
    copySSBO(buffer->colorGBufferSSBO, colorGBufferSSBO, framesize * 3);
    copySSBO(buffer->normalGBufferSSBO, normalGBufferSSBO, framesize * 3);
    copySSBO(buffer->depthGBufferSSBO, depthGBufferSSBO, framesize * 1);
    copySSBO(buffer->motionGBufferSSBO, motionGBufferSSBO, framesize * 2);
    copySSBO(buffer->albedoGBufferSSBO, albedoGBufferSSBO, framesize * 3);
    copySSBO(buffer->momentGBufferSSBO, momentGBufferSSBO, framesize * 2);
    copySSBO(buffer->meshIndexGBufferSSBO, meshIndexGBufferSSBO, framesize * 1);
    copySSBO(buffer->numSamplesGBufferSSBO, numSamplesGBufferSSBO, framesize * 1);
}




