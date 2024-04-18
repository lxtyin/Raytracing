//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_RESOURCEMANAGER_H
#define PATH_TRACING_RESOURCEMANAGER_H


#include "instance/Triangle.h"
#include "instance/Instance.h"
#include "instance/Scene.h"
#include "glad/glad.h"
#include <vector>
#include <string>
#include <map>
using std::string;

struct InstanceInfo {
    mat4 world2local;
    vec4 emission;
    int materialPtr;
    int emptyblock[11];
};
struct BVHNodeInfo {
    vec4 aa, bb;
    int lsIndex = -1;
    int rsIndex = -1;
    int instanceIndex = -1;
    int triangleIndex = -1;
};

class ResourceManager {
    
    std::vector<Mesh*> meshes;

    std::vector<GLuint64> textureHandlesBuffer;
    std::vector<float> materialBuffer;
    std::vector<Triangle> triangleBuffer;
    std::vector<InstanceInfo> instanceInfoBuffer;
    std::vector<BVHNodeInfo> meshBVHBuffer;
    std::vector<BVHNodeInfo> sceneBVHBuffer;
    GLuint textureHandleSSBO;
    GLuint materialSSBO;
    GLuint triangleSSBO;
    GLuint instanceInfoSSBO;
    GLuint meshBVHSSBO;
    GLuint sceneBVHSSBO;

    std::map<Instance*, int> instanceIndexMap;

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
     * reload all. O(numTriangles);
     * called when add/remove meshes, or update instance hierarchy.
     */
    void reload_scene(Scene *scene);

    /**
     * reload all except triangles. O(numMeshes);
     * called when updating materials / textures / transforms.
     */
    void reload_sceneinfos(Scene *scene);


};


#endif //PATH_TRACING_RESOURCEMANAGER_H
