#pragma once
#include "VertexBuffer.h"
class VertexBufferLayout;
class VertexArray
{
private:
	unsigned int m_RendererID;
	VertexBuffer* m_Vb = nullptr;
public:
	VertexArray();

	void AddBuffer(VertexBuffer& vb, const VertexBufferLayout& layout);

	void Bind() const;
	void Unbind() const;

	inline VertexBuffer* GetVb() { return m_Vb; }

	~VertexArray();
};

