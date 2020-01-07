#pragma once
#include <GL/glew.h>
#include <string>



class Texture_OLD
{
private:
	unsigned int m_ID;
	std::string m_FilePath;
	unsigned char* m_LocalBuffer;
	int m_Width, m_Height, m_BPP;
public:
	Texture_OLD(const std::string& path);

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetID() const { return m_ID; }

	int GetWidth() const { return m_Width; }
	int GetHeight() const { return m_Height; }

	~Texture_OLD();

};