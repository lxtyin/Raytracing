#include <fstream>
#include <iostream>
#include "loader.h"
#include "exglm.hpp"
#include "../material/RoughConductor.h"
using namespace std;

const float INTERVAL = 0.2; //水平间距
const float HISCALE = 0.02; //高度比例

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

            vector<uchar> data(width * height * channel);
            memcpy(data.data(), buff, data.size());
            stbi_image_free(buff);
            return new Texture(width, height, channel, move(data));
        } else {
            // local image file.
            string path = directory + name;
            return new Texture(path);
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
            result->alpha = std::max(0.001f, res);
        }
//        if(AI_SUCCESS == mat->Get(AI_MATKEY_METALLIC_FACTOR, res)){
//            result->metallic = res;
//        }
//        if(AI_SUCCESS == mat->Get(AI_MATKEY_SPECULAR_FACTOR, res)){
//            result->specular = res;
//        }

//        if(AI_SUCCESS == mat->Get(AI_MATKEY_COLOR_EMISSIVE, color)){
//            result->emission = vec3(color.r, color.g, color.b);
////            if(glm::length(result->emission) > 1) {
////                result->is_emit = true;
////            }
//        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE_ROUGHNESS, 0), str)) {
//            result->roughness_map = processImage(str.C_Str(), scene);
        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_METALNESS, 0), str)) {
//            result->metalness_map = processImage(str.C_Str(), scene);
        }
//        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), str)){
//            result->normal_map = processImage(str.C_Str(), scene);
//        }
        if(AI_SUCCESS == mat->Get(AI_MATKEY_TEXTURE(aiTextureType_BASE_COLOR, 0), str)){
            result->albedo_map = processImage(str.C_Str(), scene);
        }

        return result;
    }

    Mesh* processMesh(aiMesh *mesh, const aiScene *scene) {
        Mesh *result = new Mesh;
        result->name = mesh->mName.C_Str();

        vector<vec3> vertexs;
        vector<vec2> uvs;
        vector<vec3> normals;

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
            result->triangles.emplace_back(vertexs[id[0]], vertexs[id[1]], vertexs[id[2]],
                                           normals[id[0]], normals[id[1]], normals[id[2]],
                                           uvs[id[0]], uvs[id[1]], uvs[id[2]]);
        }

        if(mesh->mMaterialIndex >= 0){
            aiMaterial *mat = scene->mMaterials[mesh->mMaterialIndex];
            result->material = processMaterial(mat, scene);
        }
        return result;
    }

    Instance* processNode(aiNode *node, const aiScene *scene) {
        Instance *cur = new Instance;
        cur->name = node->mName.C_Str();

        aiVector3D position, rotation, scale;
        node->mTransformation.Decompose(scale, rotation, position);
        cur->transform.position = vec3(position.x, position.y, position.z);
        cur->transform.rotation = vec3(rotation.x, rotation.y, rotation.z);
        cur->transform.scale    = vec3(scale.x,    scale.y,    scale.z    );

        for(int i = 0; i < node->mNumMeshes; i++) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            cur->meshes.push_back(processMesh(mesh, scene));
        }
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

