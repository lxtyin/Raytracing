//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "Scene.h"
#include "RenderPass.h"
#include <map>

class Renderer: public RenderPass {

    vector<vec3> material_buff;
    vector<vec3> triangle_buff;
    vector<vec3> bvhnodes_buff;
    vector<vec3> lightidx_buff;
    uint material_texbuff = 0;
    uint triangle_texbuff = 0;
    uint bvhnodes_texbuff = 0;
    uint lightidx_texbuff = 0;
    int light_t_num, triangle_num;

    std::map<Triangle*, int> triangle_index;
public:

    explicit Renderer(const string &frag_shader_path, bool to_screen = false): RenderPass(frag_shader_path, to_screen) {}

    /// 生成texture buffer object
    /// \param buff 要加载的buff
    uint gen_buffer_texture(vector<vec3> &buff);

    void reload_material(Scene *scene);

    void reload_triangles(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);

    uint draw() override;
};



#endif //PATH_TRACING_RENDERER_H
