//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_RENDERPASS_H
#define PATH_TRACING_RENDERPASS_H

#include "glad/glad.h"
#include "../SSBOBuffer.h"
#include <string>
using std::string;

class RenderPass {
public:
    GLuint shaderProgram = 0;

    void use();

    /** 将texture绑定到对应的uniform sampler变量上 (old, tmp used)
     * \param name uniform variable
     * \param textureObject
     * \param targetId GL_TEXTURE0 + id
     */
    void bind_texture(const char *name, GLuint textureObject, int targetId, int type = GL_TEXTURE_2D);
};

class ComputeRenderPass: public RenderPass {
protected:
    ComputeRenderPass(const string &computeShaderPath);
    void init_shader(const string &computeShaderPath);

    void drawcall();
};

class VertexFragmentRenderPass: public RenderPass {
protected:
    GLuint VAO = 0, VBO = 0;
public:
    VertexFragmentRenderPass(const string &fragShaderPath);
    ~VertexFragmentRenderPass();

    void init_shader(const string &fragShaderPath);

    void drawcall();
};


#endif //PATH_TRACING_RENDERPASS_H
