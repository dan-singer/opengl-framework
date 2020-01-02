#include "Renderer.h"

void GLClearError()
{
	while (glGetError());
}

bool GLLogCall(const char* function, const char* file, int line)
{
	if (GLenum error = glGetError())
	{
		std::cout << "[OpenGL Error] { Code: " << error << " At function: " << function << " At line: " << line << " }\n";
		return false;
	}
	return true;
}

void Renderer::Clear() const
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const
{
	shader.Bind();
	va.Bind();
	ib.Bind();
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr); // nullptr is ok since GL_ELEMENT_ARRAY_BUFFER is bound
}
