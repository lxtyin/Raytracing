//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_RESOURCEMANAGER_H
#define PATH_TRACING_RESOURCEMANAGER_H


#include "instance/Triangle.h"
#include "glad/glad.h"
#include <vector>
#include <string>
#include <map>
using std::string;
using uint = unsigned int;

class Texture;
class Instance;
class Mesh;
class Scene;
class BVHNode;

struct InstanceInfo {
    mat4 world2local;
    int materialPtr;
    int meshIndex;
    int emptyblock[10];
};
struct BVHNodeInfo {
    vec4 aa, bb;
    int lsIndex = -1;
    int rsIndex = -1;
    int instanceIndex = -1;
    int triangleIndex = -1;
};

class ResourceManager {
    
    std::vector<Texture*> textures;
    std::vector<Mesh*> meshes;
    std::vector<std::pair<Instance*, mat4>> globalInstances; // Instances and transform to global.

    std::map<Texture*, uint> textureIndexMap;
    std::map<Mesh*, uint> meshIndexMap;
    std::map<Triangle*, uint> triangleIndexMap;
    std::map<Instance*, uint> instanceIndexMap;

    std::vector<GLuint64> textureHandlesBuffer;
    std::vector<float> materialBuffer;
    std::vector<InstanceInfo> instanceInfoBuffer;
    std::vector<Triangle> triangleBuffer;
    std::vector<BVHNodeInfo> meshBVHBuffer;
    std::vector<BVHNodeInfo> sceneBVHBuffer;

    /**
     * reload all instance info (material buffer, mesh ptr) in O(numInstances).
     */
    void reload_instances();

    /**
     * reload sceneBVH in O(numMeshes).
     */
    void reload_sceneBVH(BVHNode *root);

public:
    static ResourceManager *manager;
    ResourceManager();

    GLuint textureHandleSSBO;
    GLuint materialSSBO;
    GLuint triangleSSBO;
    GLuint instanceInfoSSBO;
    GLuint meshBVHSSBO;
    GLuint sceneBVHSSBO;

    void reload_textures();
    void reload_meshes();
    void reload_scene(Scene *scene);

    void add_texture(Texture *y);
    void del_texture(Texture *y);
    void add_mesh(Mesh *y);
    void del_mesh(Mesh *y);
    void update_globalinstance(Scene* scene);
    void update_globalinstance_recursive(Instance* ins, mat4 transform2world);

    int queryInstanceIndex(Instance *ins);
};


#endif //PATH_TRACING_RESOURCEMANAGER_H
