#include <fstream>
#include <iostream>
#include "loader.h"
#include "exglm.hpp"
#include "../material/RoughConductor.h"
#include "../material/RoughDielectric.h"
#include "../ResourceManager.h"
#include "common.h"
using namespace std;


namespace AssimpLoader{

    //loading directory
    string directory;

    Texture* processImage(const string &name, const aiScene *scene){
        int width, height, channel;
        if(name[0] == '*'){
            // embedded texture
            auto tex = scene->GetEmbeddedTexture(name.c_str());
            int siz = tex->mHeight ? tex->mWidth * tex->mHeight : tex->mWidth; //maybe compressed
            unsigned char *buff = stbi_load_from_memory(reinterpret_cast<unsigned char*>(tex->pcData),
                                                        siz,
                                                        &width,
                                                        &height,
                                                        &channel, 0);

            std::vector<uchar> data(width * height * channel);
            memcpy(data.data(), buff, data.size());
            stbi_image_free(buff);
            Texture* texture = new Texture(width, height, channel, move(data));
            ResourceManager::manager->add_texture(texture);
            return texture;
        } else {
            // local image file.
            string path = directory + name;
            Texture* texture = new Texture(path);
            ResourceManager::manager->add_texture(texture);
            return texture;
        }
    }

    Material* processMaterial(aiMaterial *mat, const aiScene *scene){

        RoughConductor* result = new RoughConductor();
        result->name = mat->GetName().C_Str();

        // 获取各项属性
        aiColor3D color;
        aiString str;
        float res;
        if(AI_SUCCESS == mat->Get(AI_MATKEY_BASE_COLOR, color)){
            result->albedo = vec3(color.r, color.g, color.b);
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, res)){
            result->roughness = std::max(0.001f, res);
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_METALLIC_FACTOR, res)){
            result->metallic = 1.0;
        }

        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), str)){
            result->albedo_map = processImage(str.C_Str(), scene);
        }

        if(AI_SUCCESS == mat->Get(AI_MATKEY_EMISSIVE_INTENSITY, color)) {
            // TODO: emitter

        }

        return result;
    }

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene) {

        std::vector<vec3> vertexs;
        std::vector<vec2> uvs;
        std::vector<vec3> normals;
        std::vector<Triangle> triangles;

        for(int i = 0; i < mesh->mNumVertices; i++){
            float tex[2] = {0, 0};
            if(mesh->mTextureCoords[0]) { //如果有纹理坐标就取第一个 ?暂不考虑多张不同uv的纹理
                tex[0] = mesh->mTextureCoords[0][i].x;
                tex[1] = mesh->mTextureCoords[0][i].y;
            }
            vertexs.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            uvs.emplace_back(tex[0], tex[1]);
        }
        for(int i = 0; i < mesh->mNumFaces; i++){
            aiFace &face = mesh->mFaces[i];
            assert(face.mNumIndices == 3);
            auto *id = face.mIndices;
            triangles.emplace_back(vertexs[id[0]], vertexs[id[1]], vertexs[id[2]],
                                           normals[id[0]], normals[id[1]], normals[id[2]],
                                           uvs[id[0]], uvs[id[1]], uvs[id[2]]);
        }

        Mesh *result = new Mesh(mesh->mName.C_Str(), std::move(triangles));
        ResourceManager::manager->add_mesh(result);
        return result;
    }

    Instance* processNode(aiNode *node, const aiScene *scene) {
        Instance *cur = new Instance;
        cur->name = node->mName.C_Str();

        aiVector3D position, rotation, scale;
        node->mTransformation.Decompose(scale, rotation, position);
        cur->transform.position = vec3(position.x, position.y, position.z);
        cur->transform.rotation = 180.0f / M_PI * vec3(rotation.x, rotation.y, rotation.z);
        cur->transform.scale    = vec3(scale.x,    scale.y,    scale.z    );

        assert(node->mNumMeshes <= 1);
        if(node->mNumMeshes == 1) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[0]];
            cur->mesh = processMesh(mesh, scene);

            if(mesh->mMaterialIndex >= 0){
                aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
                cur->material = processMaterial(mat, scene);
            }

        }
//        for(int i = 0; i < node->mNumMeshes; i++) {
//            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
//            cur->meshes.push_back(processMesh(mesh, scene));
//        }
        for(int i = 0; i < node->mNumChildren; i++) {
            cur->add_child(processNode(node->mChildren[i], scene));
        }
        return cur;
    }

    Instance* load_model(const string &path){
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate |
                                                     aiProcess_FlipUVs |
                                                     aiProcess_GenNormals |
                                                     aiProcess_RemoveRedundantMaterials);
        // 第二个参数为预处理指令，此处指定：全部转换为三角形 | 翻转纹理y轴 | 自动生成法线

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
            std::cerr<<"ERROR::ASSIMP::"<<import.GetErrorString()<<'\n';
            return nullptr;
        }
        directory = path;
        while(!directory.empty() && directory.back() != '/' && directory.back() != '\\') directory.pop_back();

        Instance *result = processNode(scene->mRootNode, scene);
        // 有时会莫名整个旋转
        result->transform.scale = vec3(1);
        result->transform.rotation = vec3(0);
        result->transform.position = vec3(0);
        return result;
    }
}

