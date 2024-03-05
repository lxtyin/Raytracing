//
// Created by lx_tyin on 2023/2/23.
//

#ifndef PATH_TRACING_RENDERPASS_H
#define PATH_TRACING_RENDERPASS_H

#include "instance/Scene.h"
#include "glad/glad.h"
#include <string>
using std::string;
using uint = unsigned int;

class RenderPass {
protected:
    uint tex_unit = 0; // 使用到的纹理单元，0-16
    uint VAO = 0, VBO = 0, FBO = 0;

    void init_canvas();
    void init_attachment(int attach_num, bool to_screen);
    void init_shader(const string &frag_shader_path);

public:

    std::vector<uint> attachments;
    std::vector<uint> attach_textures;
    uint shaderProgram = 0;

	// set attach_num = 0 when to_screen is true, or there's inexplicable bug.
    explicit RenderPass(const string &frag_shader_path, int attach_num = 0, bool to_screen = false);

    /** 将texture绑定到对应的uniform sampler变量上
     * \param target uniform variable
     * \param tex texture id
     */
    void bind_texture(const char *target, uint tex, int type = GL_TEXTURE_2D);

    /**
     * Useage: 先use，传入uniform变量，再draw
     */
    virtual void use();
    virtual void draw();
};


#endif //PATH_TRACING_RENDERPASS_H
