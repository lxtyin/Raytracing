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


Renderer::Renderer(const string &shaderPath): VertexFragmentRenderPass(shaderPath),
                                              colorGBufferSSBO(SCREEN_W * SCREEN_H * 3),
                                              motionGBufferSSBO(SCREEN_W * SCREEN_H * 2),
                                              albedoGBufferSSBO(SCREEN_W * SCREEN_H * 3),
                                              momentGBufferSSBO(SCREEN_W * SCREEN_H * 2),
                                              numSamplesGBufferSSBO(SCREEN_W * SCREEN_H * 1)
{}


void Renderer::draw(SSBOBuffer<float> &depthGBufferSSBO,
                    SSBOBuffer<float> &normalGBufferSSBO,
                    SSBOBuffer<float> &uvGBufferSSBO,
                    SSBOBuffer<float> &instanceIndexGBufferSSBO) {

    ResourceManager::manager->textureHandleSSBO.bind_current_shader(0);
    ResourceManager::manager->materialSSBO.bind_current_shader(1);
    ResourceManager::manager->triangleSSBO.bind_current_shader(2);
    ResourceManager::manager->instanceInfoSSBO.bind_current_shader(3);
    ResourceManager::manager->meshBVHSSBO.bind_current_shader(4);
    ResourceManager::manager->sceneBVHSSBO.bind_current_shader(5);

    colorGBufferSSBO.bind_current_shader(6);
    motionGBufferSSBO.bind_current_shader(7);
    albedoGBufferSSBO.bind_current_shader(8);
    momentGBufferSSBO.bind_current_shader(9);
    numSamplesGBufferSSBO.bind_current_shader(10);

    depthGBufferSSBO.bind_current_shader(11);
    normalGBufferSSBO.bind_current_shader(12);
    uvGBufferSSBO.bind_current_shader(13);
    instanceIndexGBufferSSBO.bind_current_shader(14);

    drawcall();
}

Renderer::~Renderer() {
    colorGBufferSSBO.release();
    motionGBufferSSBO.release();
    albedoGBufferSSBO.release();
    momentGBufferSSBO.release();
    numSamplesGBufferSSBO.release();
}


