#pragma once
class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_VertexCount;
public:
	VertexBuffer(const void* data, unsigned int size, unsigned int vertexCount);

	void Bind() const;
	void Unbind() const;

	inline unsigned int GetVertexCount() { return m_VertexCount; }

	~VertexBuffer();
};

