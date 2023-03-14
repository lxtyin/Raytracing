//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "instance/Scene.h"
#include "RenderPass.h"
#include "texture/Texture.h"
#include <map>

#define M_SIZ 7
#define T_SIZ 9
#define B_SIZ 3

class Renderer: public RenderPass {

    vector<vec3> material_buff;
    vector<vec3> triangle_buff;
    vector<vec3> bvhnodes_buff;
    vector<vec3> lightidx_buff;
    uint material_texbuff = 0;
    uint triangle_texbuff = 0;
    uint bvhnodes_texbuff = 0;
    uint lightidx_texbuff = 0;
    int light_num, triangle_num, material_num;
    vector<Texture*> texture_list;

    std::map<Triangle*, int> triangle_index;
public:

    Renderer(const string &frag_shader_path, int attach_num = 0, bool to_screen = false):
        RenderPass(frag_shader_path, attach_num, to_screen) {}

    /**
     * 生成texture buffer object
     * @param buff 要加载的buff
     * @return
     */
    uint gen_buffer_texture(vector<vec3> &buff);

    void reload_meshes(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);

    void draw() override;
};



#endif //PATH_TRACING_RENDERER_H
