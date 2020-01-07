#include "IndexBuffer.h"
#include <GL/glew.h>

IndexBuffer::IndexBuffer(const unsigned int* data, unsigned int count) 
	: m_Count(count)
{
	glGenBuffers(1, &m_RendererID); // Generates data that will go on GPU
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); // What type of buffer should this act as?
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Count * sizeof(unsigned int), data, GL_STATIC_DRAW); // Copy the data to the GPU
}

void IndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID); 
}

void IndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

IndexBuffer::~IndexBuffer() 
{
	glDeleteBuffers(1, &m_RendererID);
}
