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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Draw(VertexArray& va, const Shader& shader) const
{
	shader.Bind();
	va.Bind();
	glDrawArrays(GL_TRIANGLES, 0, va.GetVb()->GetSize());
}
