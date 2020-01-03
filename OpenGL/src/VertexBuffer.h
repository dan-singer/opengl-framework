#pragma once
class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Size;
public:
	VertexBuffer(const void* data, unsigned int size);

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetSize() { return m_Size; }

	~VertexBuffer();
};

