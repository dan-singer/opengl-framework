#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_RendererID);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout)
{
	Bind();
	vb.Bind();
	const auto& attributes = layout.GetAttributes();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < attributes.size(); ++i) 
	{
		const auto& attribute = attributes[i];
		glEnableVertexAttribArray(i); // Enable attribute i
		glVertexAttribPointer(
			i,								// Which attribute are we talking about?
			attribute.count,				// How many components are in this attribute?
			attribute.type,					// What's the data type?
			attribute.normalized,			// Should we normalize?
			layout.GetStride(),				// Stride: How many bytes to get to the next *vertex*
			(const void*)offset				// Pointer: How many bytes to get to this *attribute* from the first *attribute*
		);
		offset += attribute.count * VertexBufferAttribute::GetSizeOfType(attribute.type);
	}

}

void VertexArray::Bind() const
{
	glBindVertexArray(m_RendererID);
}

void VertexArray::Unbind() const
{
	glBindVertexArray(0);
}

VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_RendererID);
}
