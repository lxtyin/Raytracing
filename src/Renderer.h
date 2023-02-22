//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "Scene.h"
#include <map>

class Renderer {
    unsigned int shaderProgram = -1;
    unsigned int VAO, VBO;

    unsigned int frame;

    vector<vec3> material_buff;
    vector<vec3> triangle_buff;
    vector<vec3> bvhnodes_buff;
    vector<vec3> lightindex_buff;    // 发光triangle的index <(index, 0, 0)>
    std::map<Triangle*, int> triangle_index;
public:

    void init();

    /// 将某buff加载到shader
    /// \param buff 要加载的buff
    /// \param name 加载到哪个sampleBuff上
    /// \param idx 用的Texture单元
    void load_to_gpu(vector<vec3> &buff, const char *name, int idx);

    void reload_material(Scene *scene);

    void reload_triangles(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);

    void set_screen(int w, int h);

    /// 绘制一帧
    /// \param view 相机矩阵(v2w)
    void draw(Scene *scene, mat4 view);

};



#endif //PATH_TRACING_RENDERER_H
