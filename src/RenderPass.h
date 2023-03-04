//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_RENDERPASS_H
#define PATH_TRACING_RENDERPASS_H

#include "Scene.h"
#include "glad/glad.h"
#include <string>
using std::string;
using uint = unsigned int;

class RenderPass {
protected:
public:
    uint tex_unit = 0; // 使用到的纹理单元，0-11，后面的保留用作GL_TEXTURE_BUFFER
    uint VAO = 0, VBO = 0, FBO = 0, FBO_TEX = 0;

    uint shaderProgram = 0;

    explicit RenderPass(const string &frag_shader_path, bool to_screen = false);

    /// 将texture绑定到对应的uniform sampler变量上
    /// \param target uniform variable
    /// \param tex texture id
    void bind_texture(const char *target, uint tex, int type = GL_TEXTURE_2D);

    /// usage：use后设置，然后draw
    virtual void use();

    virtual uint draw();
};


#endif //PATH_TRACING_RENDERPASS_H
