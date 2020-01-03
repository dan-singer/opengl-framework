#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void* data, unsigned int size) : 
	m_Size(size)
{
	glGenBuffers(1, &m_RendererID); // Generates data that will go on GPU
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); // What type of buffer should this act as?
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW); // Copy the data to the GPU
}

void VertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_RendererID); // What type of buffer should this act as?
}

void VertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0); // What type of buffer should this act as?
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &m_RendererID);
}
