//
// Created by 19450 on 2024/3/17.
//

#include "GBuffer.h"
#include "Config.h"

GBuffer::GBuffer() {
    glGenBuffers(1, &colorGBufferSSBO);
    glGenBuffers(1, &normalGBufferSSBO);
    glGenBuffers(1, &depthGBufferSSBO);
    glGenBuffers(1, &motionGBufferSSBO);
    glGenBuffers(1, &albedoGBufferSSBO);
    glGenBuffers(1, &momentGBufferSSBO);
    glGenBuffers(1, &meshIndexGBufferSSBO);

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
    glBufferData(GL_SHADER_STORAGE_BUFFER, framesize * 2 * sizeof(float), placeholder, GL_DYNAMIC_COPY);


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
}



