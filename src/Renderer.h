//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "instance/Scene.h"
#include "RenderPass.h"
#include "texture/Texture.h"
#include <map>

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


    /**
     * 当前渲染目标
     * 渲染时需要从scene中提取出所有的mesh，随后基于此构建要传递给GPU的信息
     * 在场景更新（增删mesh，改变instance的父子结构）时重构
     */
    std::vector<std::pair<Mesh*, mat4>> targetMeshes;

    std::vector<GLuint64> textureHandlesBuffer;
    std::vector<float> materialBuffer;
    std::vector<Triangle> triangleBuffer;
    std::vector<MeshInfo> meshInfoBuffer;
    std::vector<BVHNodeInfo> meshBVHBuffer;
    std::vector<BVHNodeInfo> sceneBVHBuffer;
    GLuint textureHandleSSBO;
    GLuint materialSSBO;
    GLuint triangleSSBO;
    GLuint meshInfoSSBO;
    GLuint meshBVHSSBO;
    GLuint sceneBVHSSBO;
    std::map<Mesh*, uint> meshIndexMap;
    std::map<Triangle*, uint> triangleIndexMap;


    // old
    std::vector<vec3> lightidx_buff;
    uint lightidx_texbuff = 0;
    int light_num;

public:

    Renderer(const string &frag_shader_path, int attach_num = 0, bool to_screen = false);


    /**
     * 生成texture buffer object (old code)
     * @param buff 要加载的buff
     * @return
     */
    uint gen_buffer_texture(std::vector<vec3> &buff);


    void reload_materials(Scene *scene);

    void reload_transforms(Scene *scene);

    void reload_meshes(Scene *scene);

    void reload_bvhnodes(Scene *scene);

    /// 重新将scene中的内容加载到buff
    void reload_scene(Scene *scene);

    void draw() override;
};



#endif //PATH_TRACING_RENDERER_H
