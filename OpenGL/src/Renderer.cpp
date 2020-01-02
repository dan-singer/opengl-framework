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