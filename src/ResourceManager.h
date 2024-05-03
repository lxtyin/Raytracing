//
// Created by 19450 on 2024/3/4.
//

#ifndef PATH_TRACING_RESOURCEMANAGER_H
#define PATH_TRACING_RESOURCEMANAGER_H


#include "glad/glad.h"
#include "instance/Triangle.h"
#include "SSBOBuffer.h"
#include <vector>
#include <string>
#include <map>
using std::string;
using uint = unsigned int;

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
class LightInfo {
public:
    // temporal used. simple light
    int type;
    float x, y, z;
    float r, g, b;
    float emptyblock;
    LightInfo(int t, vec3 p, vec3 v) {
        type = t;
        x = p.x, y = p.y, z = p.z;
        r = v.x, g = v.y, b = v.z;
    }
};

class ResourceManager {

    std::vector<Texture*> textures;
    std::vector<Mesh*> meshes;
    std::vector<std::pair<Instance*, mat4>> globalInstances;

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
    std::vector<LightInfo> lightBuffer;

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

    ~ResourceManager();

    SSBOBuffer<GLuint64> textureHandleSSBO;
    SSBOBuffer<float> materialSSBO;
    SSBOBuffer<Triangle> triangleSSBO;
    SSBOBuffer<InstanceInfo> instanceInfoSSBO;
    SSBOBuffer<BVHNodeInfo> meshBVHSSBO;
    SSBOBuffer<BVHNodeInfo> sceneBVHSSBO;
    SSBOBuffer<LightInfo> lightSSBO;

    void reload_textures();
    void reload_meshes();
    void reload_scene(Scene *scene);

    void add_texture(Texture *y);
    void del_texture(Texture *y);
    void add_mesh(Mesh *y);
    void del_mesh(Mesh *y);
    void update_globalinstance(Scene* scene);
    void update_globalinstance_recursive(Instance* ins, mat4 transform2world);

    std::vector<std::pair<Instance*, mat4>> getGlobalInstances();

    int queryInstanceIndex(Instance *ins);
    int getLightCount();
};


#endif //PATH_TRACING_RESOURCEMANAGER_H
