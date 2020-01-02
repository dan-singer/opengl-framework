#include "Texture.h"

Texture::Texture(const std::string& path) :
	m_RendererID(0), m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_BPP(0)
{

}

void Texture::Bind(unsigned int slot /*= 0*/) const
{

}

void Texture::Unbind() const
{

}

Texture::~Texture()
{

}

