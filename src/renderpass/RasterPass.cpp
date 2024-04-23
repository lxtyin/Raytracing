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

void RasterPass::draw(Camera *camera, vec2 jitter) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    GLuint attaches[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, attaches);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1000000.0f, 1000000.0f, 1000000.0f, 1000000.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    auto globalInstances = ResourceManager::manager->getGlobalInstances();
    mat4 proj = camera->projection();
    proj[2][0] += (jitter.x - 0.5f) * 2.0f / SCREEN_W;
    proj[2][1] += (jitter.y - 0.5f) * 2.0f / SCREEN_H;

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

//    copy_fbodata_to_ssbo();
}

RasterPass::RasterPass(const string &vertexShaderPath, const string &fragShaderPath) {
    init_shader(vertexShaderPath, fragShaderPath);
    init_fbo();
}

RasterPass::~RasterPass() {
    glDeleteFramebuffers(1, &frameBufferObject);
    glDeleteTextures(1, &depthGBufferTexture);
    glDeleteTextures(1, &normalGBufferTexture);
    glDeleteTextures(1, &uvGBufferTexture);
    glDeleteTextures(1, &instanceIndexGBufferTexture);
}

void RasterPass::init_fbo() {
    glGenFramebuffers(1, &frameBufferObject);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

    // Generate texture
    glGenTextures(1, &depthGBufferTexture);
    glBindTexture(GL_TEXTURE_2D, depthGBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, Config::WINDOW_W, Config::WINDOW_H, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthGBufferTexture, 0);

    glGenTextures(1, &normalGBufferTexture);
    glBindTexture(GL_TEXTURE_2D, normalGBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, Config::WINDOW_W, Config::WINDOW_H, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalGBufferTexture, 0);

    glGenTextures(1, &uvGBufferTexture);
    glBindTexture(GL_TEXTURE_2D, uvGBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, Config::WINDOW_W, Config::WINDOW_H, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, uvGBufferTexture, 0);

    glGenTextures(1, &instanceIndexGBufferTexture);
    glBindTexture(GL_TEXTURE_2D, instanceIndexGBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, Config::WINDOW_W, Config::WINDOW_H, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, instanceIndexGBufferTexture, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &depthTestRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthTestRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, Config::WINDOW_W, Config::WINDOW_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthTestRenderBuffer);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "RasterPass: fail to create fbo." << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//[[maybe_unused]] void RasterPass::copy_fbodata_to_ssbo() {
//    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
//    float *data = new float[SCREEN_W * SCREEN_H * 3];
//
//    glReadBuffer(GL_COLOR_ATTACHMENT0);     // depth
//    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RED, GL_FLOAT, data);
//    depthGBufferSSBO.copy(data);
//
//    glReadBuffer(GL_COLOR_ATTACHMENT1);     // normal
//    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RGB, GL_FLOAT, data);
//    normalGBufferSSBO.copy(data);
//
//    glReadBuffer(GL_COLOR_ATTACHMENT2);     // uv
//    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RG, GL_FLOAT, data);
//    uvGBufferSSBO.copy(data);
//
//    glReadBuffer(GL_COLOR_ATTACHMENT3);     // instanceIndex
//    glReadPixels(0, Config::WINDOW_H - SCREEN_H, SCREEN_W, SCREEN_H, GL_RED, GL_FLOAT, data);
//    instanceIndexGBufferSSBO.copy(data);
//
//    delete[] data;
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
