#include "Mesh.h"

void Mesh::SetupMesh()
{
	// Setup the vertex array
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	// Setup the vertex buffer
	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		m_Vertices.size() * sizeof(Vertex),
		(const void*)&m_Vertices[0],
		GL_STATIC_DRAW
	);

	// Setup the index buffer
	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		m_Indices.size() * sizeof(unsigned int),
		(const void*)& m_Indices[0],
		GL_STATIC_DRAW
	);

	// Setup the vertex buffer layout
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, normal));
	// Vertex Texture Coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoords));
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture*> textures) :
	m_Vertices(vertices), m_Indices(indices), m_Textures(textures)
{
	SetupMesh();
}

void Mesh::Draw(Shader& shader)
{
	unsigned int diffuseNum = 0;
	unsigned int specNum = 0;
	for (unsigned int i = 0; i < m_Textures.size(); ++i)
	{
		std::string number;
		std::string typeName = m_Textures[i]->GetTypeString();
		switch (m_Textures[i]->GetType())
		{
		case aiTextureType_DIFFUSE:
			number = std::to_string(diffuseNum++);
			break;
		case aiTextureType_SPECULAR:
			number = std::to_string(specNum++);
			break;
		}
		shader.SetUniform1i("material.texture_" + typeName + number, i);
		m_Textures[i]->Bind(i);
	}
	glActiveTexture(GL_TEXTURE0);

	// Draw Mesh
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}



