//
// Created by lx_tyin on 2023/2/23.
//

#include "Config.h"
#include "RenderPass.h"
#include "tool/tool.h"
#include "glad/glad.h"
#include <iostream>

RenderPass::RenderPass(const string &frag_shader_path, bool to_screen) {

    // 方形屏幕
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

    // 生成shader
    string vertexShaderCode = read_file("shader/fullscreen.vert");
    string fragmentShaderCode = read_file(frag_shader_path);
    const char *vertexCodePointer = vertexShaderCode.c_str();
    const char *fragmentCodePointer = fragmentShaderCode.c_str();

    int success;
    char infoLog[512];
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexCodePointer, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "RenderPass: vertexShader compile failed: " << infoLog << std::endl;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentCodePointer, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "RenderPass: " << frag_shader_path << " compile failed: " << infoLog << std::endl;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success){
        glGetShaderInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "RenderPass: link failed: " << infoLog << std::endl;
    }

    glUseProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 帧缓冲
    if(to_screen) FBO = 0, FBO_TEX = 0;
    else{
        glGenFramebuffers(1, &FBO);
        glGenTextures(1, &FBO_TEX);
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        glBindTexture(GL_TEXTURE_2D, FBO_TEX);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCREEN_W, SCREEN_H, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBO_TEX, 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "RenderPass: framebuffer uncompleted." << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}

void RenderPass::bind_texture(const char *target, uint tex, int type) {
    glActiveTexture(GL_TEXTURE0 + tex_unit);
    glBindTexture(type, tex);
    glUniform1i(glGetUniformLocation(shaderProgram, target), tex_unit);
    ++tex_unit;
    assert(tex_unit < 16);
}

void RenderPass::use() {
    glUseProgram(shaderProgram);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glBindVertexArray(VAO);
    tex_unit = 0;
}

uint RenderPass::draw() {
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    return FBO_TEX;
}