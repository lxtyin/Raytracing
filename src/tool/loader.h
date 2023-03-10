#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../instance/Mesh.h"
#include "../texture/Texture.h"
#include "../instance/Instance.h"
#include "../instance/Material.h"

namespace Loader{

	Texture*  processImage(const string &name, const aiScene *scene);
	Material* processMaterial(aiMaterial *mat, const aiScene *scene);
	Instance* processNode(aiNode *node, const aiScene *scene, Instance *t_node);
    Mesh* processMesh(aiMesh *mesh, const aiScene *scene, Instance *t_node);
	Instance* load_model(const string &path);

}




