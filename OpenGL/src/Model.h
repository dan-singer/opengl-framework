#pragma once
#include "Mesh.h"
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.h"
#include <map>

class Model {
private:
	std::vector<Mesh> m_Meshes;
	std::map<std::string, Texture*> m_LoadedTextures;
	std::string m_Directory;
private:
	void LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture*> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
public:
	Model(const char* path);
	void Draw(Shader& shader, unsigned int skybox = 0);

	// TODO implement proper destructor!
};