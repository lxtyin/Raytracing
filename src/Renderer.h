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

    struct MeshInfo {
        mat4 world2local;
        vec4 emission;
        int materialPtr;
        int emptyblock[11];
    };

    struct BVHNodeInfo {
        vec4 aa, bb;
        int lsIndex = -1;
        int rsIndex = -1;
        int meshIndex = -1;
        int triangleIndex = -1;
    };

    std::vector<GLuint64> textureHandlesBuffer;
    std::vector<float> materialBuffer;
    std::vector<MeshInfo> meshInfoBuffer; // TODO 传进去错位了？？？
    std::vector<BVHNodeInfo> bvhNodeBuffer;
    GLuint textureHandleSSBO;
    GLuint meshInfoSSBO;
    GLuint materialSSBO;
    GLuint bvhNodeSSBO;

    std::map<Mesh*, uint> meshIndexMap;
    std::map<Triangle*, uint> triangleIndexMap;


    // old
    std::vector<vec3> material_buff;
    std::vector<vec3> triangle_buff;
    std::vector<vec3> lightidx_buff;
    uint triangle_texbuff = 0;
    uint lightidx_texbuff = 0;
    int light_num, triangle_num;

public:

    Renderer(const string &frag_shader_path, int attach_num = 0, bool to_screen = false):
        RenderPass(frag_shader_path, attach_num, to_screen) {}

    /**
     * 生成texture buffer object
     * @param buff 要加载的buff
     * @return
     */
    uint gen_buffer_texture(std::vector<vec3> &buff);

    void reload_meshes(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);

    void draw() override;
};



#endif //PATH_TRACING_RENDERER_H
