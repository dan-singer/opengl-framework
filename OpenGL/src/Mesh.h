#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "Shader.h"
#include "Vertex.h"
#include "assimp/material.h"
#include "Texture.h"

class Mesh {
private:
	unsigned int m_VBO;
	unsigned int m_VAO;
	unsigned int m_EBO;
public:
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<Texture*> m_Textures;
private:
	void SetupMesh();
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures);
	void Draw(Shader& shader);
};