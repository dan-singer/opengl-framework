#pragma once
#include <vector>
#include <GL/glew.h>

struct VertexBufferAttribute
{
	unsigned int type;
	unsigned int count;
	unsigned char normalized;

	static unsigned int GetSizeOfType(unsigned int type)
	{
		switch (type)
		{
		case GL_FLOAT:			return 4;
		case GL_UNSIGNED_INT:	return 4;
		case GL_UNSIGNED_BYTE:	return 1;
		}
		return 0;
	}
};

class VertexBufferLayout
{
private:
	std::vector<VertexBufferAttribute> m_Attributes;
	unsigned int m_Stride;
public:
	VertexBufferLayout() :
		m_Stride(0)
	{

	}

	template<class T>
	void Push(unsigned int count)
	{
		static_assert(false);
	}

	template<>
	void Push<float>(unsigned int count)
	{
		m_Attributes.push_back({ GL_FLOAT, count, GL_FALSE });
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_FLOAT) * count;
	}

	template<>
	void Push<unsigned int>(unsigned int count)
	{
		m_Attributes.push_back({ GL_UNSIGNED_INT, count, GL_FALSE });
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_INT) * count;

	}

	template<>
	void Push<unsigned char>(unsigned int count)
	{
		m_Attributes.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE });
		m_Stride += VertexBufferAttribute::GetSizeOfType(GL_UNSIGNED_BYTE) * count;

	}

	inline unsigned int GetStride() const { return m_Stride; }
	inline const std::vector<VertexBufferAttribute>& GetAttributes() const { return m_Attributes; }

	~VertexBufferLayout()
	{

	}
};

