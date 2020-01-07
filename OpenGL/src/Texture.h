#pragma once
#include <GL/glew.h>
#include <string>
#include "assimp/material.h"

class Texture
{
private:
	unsigned int m_ID;
	aiTextureType m_Type;
	std::string m_Path;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
public:
	Texture(const std::string& path, aiTextureType type);

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetID() const { return m_ID; }

	aiTextureType GetType() { return m_Type; }
	std::string GetTypeString();

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	~Texture();

};