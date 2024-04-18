//
// Created by lx_tyin on 2023/2/20.
//

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "../Config.h"
#include "Renderer.h"
#include "../texture/Texture.h"
#include "../instance/Mesh.h"
#include "../instance/Scene.h"
#include "../material/Material.h"
#include "../BVH.h"
#include "../ResourceManager.h"
#include <queue>
#include <set>

void Renderer::draw(GBuffer &gbuffer) {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ResourceManager::manager->textureHandleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ResourceManager::manager->materialSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ResourceManager::manager->triangleSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ResourceManager::manager->instanceInfoSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ResourceManager::manager->meshBVHSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, ResourceManager::manager->sceneBVHSSBO);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, gbuffer.colorGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, gbuffer.normalGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, gbuffer.depthGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, gbuffer.motionGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, gbuffer.albedoGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, gbuffer.momentGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, gbuffer.instanceIndexGBufferSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, gbuffer.numSamplesGBufferSSBO);


    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);

    // Run compute shader
//    glDispatchCompute((SCREEN_H + 31) / 32,
//                      (SCREEN_W + 31) / 32,
//                      1);
//    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

//    cout << colorGBufferSSBO << "R: ";
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colorGBufferSSBO);
//    float* tmpdata = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, framesize * sizeof(float), GL_MAP_READ_BIT);
//    for(int i = 0;i < 10;i++) std::cout << tmpdata[i] << ' ';
//    std::cout << std::endl;
//    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

Renderer::Renderer(const string &shaderPath): VertexFragmentRenderPass(shaderPath) {}

