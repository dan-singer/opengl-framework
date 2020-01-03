#include "Shader.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
Shader::Shader(const std::string& vs, const std::string& ps) :
	m_RendererID(0)
{
	ShaderProgramSource source = ParseShader(vs, ps);
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

void Shader::Bind() const
{
	glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
	glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int i0)
{
	glUniform1i(GetUniformLocation(name), i0);
}

void Shader::SetUniform1f(const std::string& name, float v0)
{
	glUniform1f(GetUniformLocation(name), v0);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
	glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform3f(const std::string& name, glm::vec3 vector)
{
	glUniform3f(GetUniformLocation(name), vector.x, vector.y, vector.z);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
	glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& name, glm::mat4 matrix)
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

int Shader::GetUniformLocation(const std::string& name)
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
	{
		return m_UniformLocationCache[name];
	}

	int location = glGetUniformLocation(m_RendererID, name.c_str());
	if (location == -1)
	{
		std::cout << "Warning: uniform " << name << " doesn't exist!\n";
	}
	m_UniformLocationCache[name] = location;
	return location;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}



ShaderProgramSource Shader::ParseShader(const std::string& vs, const std::string& ps)
{
	std::ifstream vsStream(vs);
	std::ifstream psStream(ps);

	std::string vsSource((std::istreambuf_iterator<char>(vsStream)), std::istreambuf_iterator<char>());
	std::string psSource((std::istreambuf_iterator<char>(psStream)), std::istreambuf_iterator<char>());
	return { vsSource, psSource };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char)); // Allocate dynamically sized data on the stack!
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!\n";
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

Shader::~Shader()
{
	glDeleteProgram(m_RendererID);
}


