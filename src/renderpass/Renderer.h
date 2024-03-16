//
// Created by lx_tyin on 2023/2/20.
//

#ifndef PATH_TRACING_RENDERER_H
#define PATH_TRACING_RENDERER_H

#include "../instance/Scene.h"
#include "../texture/Texture.h"
#include "RenderPass.h"
#include <map>


class Renderer: public VertexFragmentRenderPass {
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

    /**
     * reload meshInfos in O(numMeshes). Including transforms, materials/textures
     */
    void reload_meshInfos(Scene* scene);

    /**
     * reload all triangles and meshBVH in O(numTriangles).
     */
    void reload_triangles(Scene *scene);

    /**
     * reload sceneBVH in O(numMeshes). Called when objects moved.
     */
    void reload_sceneBVH(Scene *scene);
public:
    /**
     * GBuffers, store in texture order (Down is x and right is y).
     */
    GLuint colorBufferSSBO;
    GLuint normalBufferSSBO;
    GLuint positionBufferSSBO;

    Renderer(const string &shaderPath);

    /**
     * reload all. O(numTriangles);
     * called when add/remove meshes, or update instance hierarchy.
     */
    void reload_scene(Scene *scene);

    /**
     * reload all except triangles. O(numMeshes);
     * called when updating materials / textures / transforms.
     */
    void reload_sceneinfos(Scene *scene);


    void draw();
};


#endif //PATH_TRACING_RENDERER_H
