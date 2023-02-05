#include <utils.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <array>
#include <glm/gtc/type_ptr.hpp>

shader::shader(const char* vertex_path, const char* fragment_path, const char* geometry_path)
{
	std::string vertex_code{ read_file(vertex_path) };
	char const* vc = vertex_code.data();
	unsigned vertex_id = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_id, 1, &vc, NULL);
	glCompileShader(vertex_id);
	check_compile_status(vertex_id, "VERTEX");

	std::string fragment_code{ read_file(fragment_path) };
	char const* fc = fragment_code.data();
	unsigned fragment_id = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_id, 1, &fc, NULL);
	glCompileShader(fragment_id);
	check_compile_status(fragment_id, "FRAGMENT");

	m_program_id = glCreateProgram();
	glAttachShader(m_program_id, vertex_id);
	glAttachShader(m_program_id, fragment_id);

	if (geometry_path)
	{
		std::string fragment_code{ read_file(geometry_path) };
		char const* fc = fragment_code.data();
		unsigned geometry_id = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry_id, 1, &fc, NULL);
		glCompileShader(geometry_id);
		check_compile_status(geometry_id, "GEOMETRY");
		glAttachShader(m_program_id, geometry_id);
	}

	glLinkProgram(m_program_id);
	check_linking_status(m_program_id);

	glDeleteShader(vertex_id);
	glDeleteShader(fragment_id);

	use_program();
}

void shader::use_program() const
{
	glUseProgram(m_program_id);
}

void shader::set_uniform(const char* uniform_name, float f1, float f2, float f3, float f4)
{
	const int loc = glGetUniformLocation(m_program_id, uniform_name);
	glUniform4f(loc, f1, f2, f3, f4);
}

void shader::set_uniform(const char* uniform_name, float f)
{
	const int loc = glGetUniformLocation(m_program_id, uniform_name);
	glUniform1f(loc, f);
}

void shader::set_uniform(const char* uniform_name, unsigned i)
{
	const int loc = glGetUniformLocation(m_program_id, uniform_name);
	glUniform1i(loc, i);
}

void shader::set_uniform(const char* uniform_name, glm::mat4 const& m4)
{
	const int loc = glGetUniformLocation(m_program_id, uniform_name);
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m4));
}

void shader::set_uniform(const char* uniform_name, glm::vec3 const& v3)
{
	const int loc = glGetUniformLocation(m_program_id, uniform_name);
	glUniform3fv(loc, 1, &v3[0]);
}

std::string shader::read_file(const char* path) const
{
	std::ifstream file(path);
	std::stringstream buffer;
	buffer << file.rdbuf();
	const std::string& str = buffer.str();
	if (str.empty()) throw std::runtime_error("File is empty");
	return str;
}

void shader::check_compile_status(unsigned shader_id, std::string type) const
{
	int success{};
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		std::array<char, 512> infoLog;
		glGetShaderInfoLog(shader_id, static_cast<GLsizei>(infoLog.size()), NULL, infoLog.data());
		std::cout << "ERROR::SHADER::"<<type<<"::COMPILATION_FAILED\n" << infoLog.data() << "\n";
		throw std::runtime_error("Cannot compile shader\n");
	};
}

void shader::check_linking_status(unsigned program_id) const
{
	int success{};
	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success)
	{
		std::array<char, 512> infoLog;
		glGetProgramInfoLog(program_id, static_cast<GLsizei>(infoLog.size()), NULL, infoLog.data());
		std::cout << "ERROR::PROGRAM_LINKING_ERROR::" << infoLog.data() << "\n";
	}
}
