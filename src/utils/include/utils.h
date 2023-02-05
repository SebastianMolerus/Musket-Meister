#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

int load_texture(std::string const& texture_file_path);

class shader
{
public:
	shader(const char* vertex_path, const char* fragment_path, const char* geometry_path = nullptr);
	
	void use_program() const;

	void set_uniform(const char* uniform_name, float f1, float f2, float f3, float f4);
	void set_uniform(const char* uniform_name, float f);
	void set_uniform(const char* uniform_name, unsigned i);
	void set_uniform(const char* uniform_name, glm::mat4 const& m4);
	void set_uniform(const char* uniform_name, glm::vec3 const& v3);

private:
	unsigned m_program_id{};
	std::string read_file(const char* file_path) const;
	void check_compile_status(unsigned shader_id, std::string type) const;
	void check_linking_status(unsigned program_id) const;
};

class vertex_buffer
{
public:
	vertex_buffer();

	void fill_array_buffer(void const* const buffer, unsigned size);

	void fill_element_array_buffer(void const* const buffer, unsigned size);

	// always to call before draw
	void bind_vao();

	// example data : pos_x, pos_y, pos_z, color_x, color_y
	// pattern		: "32"
	void set_vertex_attrib_pointers(const char* const pattern);
private:
    unsigned m_vao{};
};

class camera
{
    public:
    camera(unsigned screen_width, unsigned screen_height);

    void move_up(float value);
    void move_down(float value);

    void move_right(float value);
    void move_left(float value);

    glm::mat4 const& calc_view_matrix();
    glm::mat4 const& get_projection_matrix() const;

	glm::vec3 const& get_camera_pos() const;

    private:
    glm::mat4 m_view;
    glm::mat4 const m_projection;
    
    glm::vec3 m_camera_pos{ 0.0f, -25.0f, 40.0f };
    glm::vec3 const m_up{0.0f, 1.0f, 0.0f};
    glm::vec3 const m_camera_direction{ glm::normalize(glm::vec3{0.0f, 1.0f, -1.0f }) };
};

#endif