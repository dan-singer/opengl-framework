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

void Mesh::Draw(Shader& shader, unsigned int skybox)
{
	unsigned int diffuseNum = 0;
	unsigned int specNum = 0;
	unsigned int maxTextureTypes = 2;
	for (unsigned int i = 0; i < m_Textures.size(); ++i)
	{
		std::string typeName = m_Textures[i]->GetTypeString();
		shader.SetUniform1i("material." + typeName, i);
		m_Textures[i]->Bind(i);
	}
	if (skybox != 0)
	{
		// We use maxTextureTypes rather than m_Textures.size()
		// If there were 0 textures, diffuse and specular would be assigned to slot 0, but so would 
		// the skybox! In that case, we'd have multiple texture types in the same texture slot!
		glActiveTexture(GL_TEXTURE0 + maxTextureTypes);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);
	}
	shader.SetUniform1i("skybox", maxTextureTypes);


	glActiveTexture(GL_TEXTURE0);

	// Draw Mesh
	glBindVertexArray(m_VAO);
	glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}



