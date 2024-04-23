//
// Created by lx_tyin on 2023/2/23.
//
#include "../Config.h"
#include "../tool/tool.h"
#include "RenderPass.h"
#include <iostream>

void ComputeRenderPass::init_shader(const string &computeShaderPath) {
    // 生成shader
    string computeShaderCode = read_shader(computeShaderPath);
    const char *computeCodePointer = computeShaderCode.c_str();
    int success;

    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(computeShader, 1, &computeCodePointer, nullptr);
    glCompileShader(computeShader);
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if(!success){
        GLchar infoLog[512];
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        std::cerr << "RenderPass: " << computeShaderPath << " compile failed: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, computeShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        GLchar infoLog[1024];
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
        std::cerr << "RenderPass: " << computeShaderPath << " link failed: " << infoLog << std::endl;
    }

    glDeleteShader(computeShader);
}

ComputeRenderPass::ComputeRenderPass(const string &computeShaderPath) {
    init_shader(computeShaderPath);
}

void ComputeRenderPass::drawcall() {
    glDispatchCompute((SCREEN_H + 31) / 32,
                      (SCREEN_W + 31) / 32,
                      1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void VertexFragmentRenderPass::init_shader(const string &fragShaderPath) {
    string vertexShaderCode = read_shader("shader/basic/fullscreen.vert");
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
        std::cerr << "RenderPass: " << fragShaderPath << " compile failed: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetProgramInfoLog(shaderProgram, 1024, nullptr, infoLog);
        save_file(fragShaderPath + ".unfold", fragmentShaderCode);
        std::cerr << "RenderPass: " << fragShaderPath << " link failed: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

VertexFragmentRenderPass::VertexFragmentRenderPass(const string &fragShaderPath) {
    init_shader(fragShaderPath);

    // use two triangles to cover the screen
    float canvas_data[] = {-1, 1, -1, -1, 1, -1,
                           1, -1, 1, 1, -1, 1};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), canvas_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);//unbind VAO
}

void VertexFragmentRenderPass::drawcall() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
}

VertexFragmentRenderPass::~VertexFragmentRenderPass() {
    glDeleteBuffers(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void RenderPass::use() {
    glUseProgram(shaderProgram);
}

void RenderPass::bind_texture(const char *name, GLuint textureObject, int targetId, int type) {
    glActiveTexture(GL_TEXTURE0 + targetId);
    glBindTexture(type, textureObject);
    glUniform1i(glGetUniformLocation(shaderProgram, name), targetId);
    assert(targetId < 32);
}
