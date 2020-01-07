#pragma once
#include "Mesh.h"
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model {
private:
	std::vector<Mesh> m_Meshes;
	std::vector<Texture> m_TexturesLoaded;
	std::string m_Directory;
private:
	unsigned int TextureFromFile(const char* name, std::string directory);
	void LoadModel(std::string path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);
public:
	Model(const char* path);
	void Draw(Shader& shader);
};