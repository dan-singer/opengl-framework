#include "Shader.h"
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
Shader::Shader(const std::string& vs, const std::string& fs, const std::string& gs /*= ""*/) :
	m_RendererID(0)
{
	ShaderProgramSource source = ParseShader(vs, fs, gs);
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource, source.GeometrySource);
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

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader)
{
	unsigned int program = glCreateProgram();

	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
	glAttachShader(program, vs);
	glAttachShader(program, fs);

	if (geometryShader != "") {
		unsigned int gs = CompileShader(GL_GEOMETRY_SHADER, geometryShader);
		glAttachShader(program, gs);
	}

	glLinkProgram(program);
	glValidateProgram(program);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}



ShaderProgramSource Shader::ParseShader(const std::string& vs, const std::string& fs, const std::string& gs /*= ""*/)
{

	ShaderProgramSource source;

	std::ifstream vsStream(vs);
	std::ifstream fsStream(fs);

	std::string vsSource((std::istreambuf_iterator<char>(vsStream)), std::istreambuf_iterator<char>());
	std::string fsSource((std::istreambuf_iterator<char>(fsStream)), std::istreambuf_iterator<char>());
	source.VertexSource = vsSource;
	source.FragmentSource = fsSource;

	if (gs != "") 
	{
		std::ifstream gsStream(gs);
		std::string gsSource((std::istreambuf_iterator<char>(gsStream)), std::istreambuf_iterator<char>());
		source.GeometrySource = gsSource;
	} 
	else
	{
		source.GeometrySource = "";
	}


	return source;
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
		std::string typeString;
		if (type == GL_VERTEX_SHADER)
			typeString = "vertex";
		else if (type == GL_FRAGMENT_SHADER)
			typeString = "fragment";
		else
			typeString = "geometry";
		std::cout << "Failed to compile " << typeString << " shader!\n";
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


