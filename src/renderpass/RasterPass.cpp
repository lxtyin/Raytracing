//
// Created by 19450 on 2024/4/20.
//

#include "../Config.h"
#include "../tool/tool.h"
#include "../instance/Instance.h"
#include "../instance/Mesh.h"
#include "../ResourceManager.h"
#include "RasterPass.h"
#include <iostream>
using glm::mat3;

void RasterPass::init_shader(const string &vertexShaderPath, const string &fragShaderPath) {
    string vertexShaderCode = read_shader(vertexShaderPath);
    string fragmentShaderCode = read_shader(fragShaderPath);
    const char *vertexCodePointer = vertexShaderCode.c_str();
    const char *fragmentCodePointer = fragmentShaderCode.c_str();

    int success;
    char infoLog[1024];
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCodePointer, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 1024, nullptr, infoLog);
        std::cerr << "RenderPass: vertexShader compile failed: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodePointer, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 1024, nullptr, infoLog);
        std::cerr << "RasterPass: " << fragShaderPath << " compile failed: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
        save_file(fragShaderPath + ".unfold", fragmentShaderCode);
        std::cerr << "RasterPass: " << fragShaderPath << " link failed: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void RasterPass::draw(Camera *camera) {
//    depthGBufferSSBO.fill(100000.0f);
//    normalGBufferSSBO.fill(0.0f);
//    uvGBufferSSBO.fill(0.0f);
//    instanceIndexGBufferSSBO.fill(-10.0);

//    depthGBufferSSBO.bind_current_shader(0);
//    normalGBufferSSBO.bind_current_shader(1);
//    uvGBufferSSBO.bind_current_shader(2);
//    instanceIndexGBufferSSBO.bind_current_shader(3);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    GLuint attaches[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attaches);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1000000.0f, 1000000.0f, 1000000.0f, 1000000.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    auto globalInstances = ResourceManager::manager->getGlobalInstances();
    mat4 proj = camera->projection();
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "proj_matrix"), 1, GL_FALSE, glm::value_ptr(proj));

    for(auto &[ins, l2w]: globalInstances) {
        mat4 view_model = camera->w2v_matrix() * l2w;
        glUniform1i(glGetUniformLocation(shaderProgram, "currentInstanceIndex"), ResourceManager::manager->queryInstanceIndex(ins));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view_model_matrix"), 1, GL_FALSE, glm::value_ptr(view_model));

        mat3 normal = l2w; // store world space normal.
        normal = glm::transpose(glm::inverse(normal));
        glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normal_matrix"), 1, GL_FALSE, glm::value_ptr(normal));

        ins->mesh->draw_in_rasterization();
    }
    glDisable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    copy_fbodata_to_ssbo();
}

RasterPass::RasterPass(const string &vertexShaderPath, const string &fragShaderPath):
        depthGBufferSSBO(SCREEN_W * SCREEN_H * 1),
        normalGBufferSSBO(SCREEN_W * SCREEN_H * 3),
        uvGBufferSSBO(SCREEN_W * SCREEN_H * 2),
        instanceIndexGBufferSSBO(SCREEN_W * SCREEN_H * 1)
{
    init_shader(vertexShaderPath, fragShaderPath);
    init_fbo();
}

RasterPass::~RasterPass() {
    depthGBufferSSBO.release();
    normalGBufferSSBO.release();
    uvGBufferSSBO.release();
    instanceIndexGBufferSSBO.release();
    glDeleteFramebuffers(1, &frameBufferObject);
    glDeleteRenderbuffers(1, &depthRenderBuffer);
    glDeleteRenderbuffers(1, &normalRenderBuffer);
    glDeleteRenderbuffers(1, &uvRenderBuffer);
    glDeleteRenderbuffers(1, &instanceRenderBuffer);
}

void RasterPass::init_fbo() {
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_R32F, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, depthRenderBuffer);

    glGenRenderbuffers(1, &normalRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, normalRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB32F, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, normalRenderBuffer);

    glGenRenderbuffers(1, &uvRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, uvRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RG32F, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, uvRenderBuffer);

    glGenRenderbuffers(1, &instanceRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, instanceRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_R32F, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_RENDERBUFFER, instanceRenderBuffer);

    glGenRenderbuffers(1, &depthTestRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthTestRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthTestRenderBuffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "RasterPass: fail to create fbo." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RasterPass::copy_fbodata_to_ssbo() {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    float *data = new float[SCREEN_W * SCREEN_H * 3];

    glReadBuffer(GL_COLOR_ATTACHMENT0);     // depth
    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RED, GL_FLOAT, data);
    depthGBufferSSBO.copy(data);

    glReadBuffer(GL_COLOR_ATTACHMENT1);     // normal
    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RGB, GL_FLOAT, data);
    normalGBufferSSBO.copy(data);

    glReadBuffer(GL_COLOR_ATTACHMENT2);     // uv
    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RG, GL_FLOAT, data);
    uvGBufferSSBO.copy(data);

    glReadBuffer(GL_COLOR_ATTACHMENT3);     // instanceIndex
    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RED, GL_FLOAT, data);
    instanceIndexGBufferSSBO.copy(data);

    delete[] data;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
