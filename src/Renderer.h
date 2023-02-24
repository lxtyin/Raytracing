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
    vector<vec3> lightindex_buff;    // 发光triangle的index <(index, 0, 0)>
    std::map<Triangle*, int> triangle_index;
public:

    explicit Renderer(const string &frag_shader_path, bool to_screen = false): RenderPass(frag_shader_path, to_screen) {}

    /// 将某buff加载到shader
    /// \param buff 要加载的buff
    /// \param name 加载到哪个sampleBuff上
    /// \param idx 用的Texture单元
    void set_buff_toshader(vector<vec3> &buff, const char *name);

    void reload_material(Scene *scene);

    void reload_triangles(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);
};



#endif //PATH_TRACING_RENDERER_H
