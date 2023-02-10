#include <iostream>
#include <array>

#include <mov.h>
#include <utils.h>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

const int g_window_width = 1920;
const int g_window_height = 1080;

camera g_cam{g_window_width, g_window_height};

glm::mat4 g_view_matrix{1.0f};

glm::vec3 g_mouse_world_pos{1.0f};

glm::mat4 const g_projection_matrix{g_cam.get_projection_matrix()};

glm::vec2 g_target_position{};
glm::vec2 g_target_orientation{};

enum class ai_mode
{
	kinematic_seek,
	kinematic_flee,
	kinematic_wander,
	kinematic_arrive,

	dynamic_seek,
	dynamic_flee,
	dynamic_arrive
} g_ai_mode;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	double const x_ndc{ (xpos / g_window_width) * 2.0f - 1 };
	double const y_ndc{ -((ypos / g_window_height) * 2.0f - 1) };

	glm::vec3 const plane_normal{ 0.0, 0.0f, 1.0f };
	glm::vec4 xy_view = glm::inverse(g_projection_matrix) * glm::vec4{ x_ndc, y_ndc, 0.0f, 0.0f };
	xy_view.z = -1;
	glm::vec3 const ray_world = glm::normalize(glm::inverse(g_view_matrix) * xy_view);
	auto const cam_pos = g_cam.get_camera_pos();
	const float t = (-glm::dot(plane_normal, cam_pos)) / (glm::dot(plane_normal, ray_world));
	g_mouse_world_pos = cam_pos + t * ray_world;
}


void mouse_click_callback(GLFWwindow*, int button, int action, int)
{
	if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
	{
		g_target_position = g_mouse_world_pos;
	}

	if(button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
	{
		g_target_orientation = glm::normalize(glm::vec2{g_mouse_world_pos.x, g_mouse_world_pos.y} - g_target_position);
	}
}

void keyboard_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch(key)
	{
		case GLFW_KEY_E:
			g_ai_mode = ai_mode::kinematic_seek;
		break;
		case GLFW_KEY_R:
			g_ai_mode = ai_mode::kinematic_flee;
		break;
		case GLFW_KEY_T:
			g_ai_mode = ai_mode::kinematic_wander;
		break;
		case GLFW_KEY_Y:
			g_ai_mode = ai_mode::kinematic_arrive;
		break;
		case GLFW_KEY_U:
			g_ai_mode = ai_mode::dynamic_seek;
		break;
		case GLFW_KEY_I:
			g_ai_mode = ai_mode::dynamic_flee;
		break;
		case GLFW_KEY_O:
			g_ai_mode = ai_mode::dynamic_arrive;
		break;
	}
}

GLFWwindow* init()
{
    glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(g_window_width, g_window_height, "Musket-Meister", NULL, NULL);
	if (!window)
	{
		std::cout << "Failed to create GLFW window\n";
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	// disable v-sync
	// glfwSwapInterval( 0 );

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_click_callback);
	glfwSetKeyCallback(window, keyboard_key_callback);
	
	stbi_set_flip_vertically_on_load(true);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		glfwTerminate();
	}

	glEnable(GL_PROGRAM_POINT_SIZE); 

	glLineWidth(2.5f);

	glEnable(GL_DEPTH_TEST);

	return window;
}

void process_input(GLFWwindow* window)
{
	// QUIT
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		g_cam.move_up(1.0f);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		g_cam.move_down(1.0f);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		g_cam.move_right(1.0f);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		g_cam.move_left(1.0f);
}

void set_window_title(GLFWwindow* window)
{
	std::string window_title {"Musket-Meister. v:"};
	{
		#ifdef __PROJECT_VERSION__
		window_title += __PROJECT_VERSION__;
		#endif
	}
	glfwSetWindowTitle(window, window_title.c_str());
}

struct background_texture
{
	int m_texture;
	shader m_shader;
	vertex_buffer m_buffer;

	const std::array<int, 6> m_indices = 
	{
	0, 1, 2,
	2, 3, 1
	};

	// 200 x 200
	const std::array<float, 20> m_vertices =
	{
	-100.0f, -100.0f, -1.0f, 0.0f, 0.0f, // bottom left
	100.0f, -100.0f, -1.0f, 200.0f, 0.0f, // bottom right
	-100.0f, 100.0f, -1.0f, 0.0f, 200.0f, // top left
	100.0f, 100.0f, -1.0f, 200.0f, 200.0f  // top right
	};

	background_texture()
	:m_texture{load_texture("./textures/grass.jpg")},
	 m_shader{"./shaders/grass_vertex.txt", "./shaders/grass_fragment.txt"}
	{
		m_buffer.fill_array_buffer(m_vertices.data(), m_vertices.size() * sizeof(float));
		m_buffer.fill_element_array_buffer(m_indices.data(), m_indices.size() * sizeof(int));
		m_buffer.set_vertex_attrib_pointers("32");
	}

	void prepare_and_draw()
	{
		m_shader.use_program();
		m_shader.set_uniform("view", g_view_matrix);
		m_shader.set_uniform("model", glm::mat4{ 1.0f });
		m_shader.set_uniform("projection", g_projection_matrix);

		m_buffer.bind_vao();
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}
};

struct green_circle
{
	const std::array<glm::vec2, 200> direction_circle() const
	{
		float const R = 0.5;
		float const dec = 2 * R / 100;

		std::array<glm::vec2, 200> res;

		// left part:
		// y = x + 0.5
		// right part:
		// y = -x + 0.5
		float x = -0.5f;
		for(unsigned i=0;i<res.size()/2;++i)
		{
			res[i].x = x;

			if(x<0.0f)
				res[i].y = x + 0.5f;
			else
				res[i].y = -1.0f * x + 0.5f;

			x += dec;
		}

		// bottom part
		float angle = 3.14;
		float const inc = 6.28 / res.size();
		for (int i = res.size() / 2; i < res.size(); ++i)
		{
			res[i].x = R * cos(angle);
			res[i].y = R * sin(angle);
			angle += inc;
		}

		return res;
	}
	
	const std::array<glm::vec2, 200> m_vertices;
	shader m_shader;
	vertex_buffer m_buffer;
	
	green_circle()
	:m_vertices{direction_circle()},
	 m_shader{ "./shaders/unit_vertex.txt", "./shaders/unit_fragment.txt" }
	{
		auto unit_vertixes = direction_circle();
		shader unit_shader{ "./shaders/unit_vertex.txt", "./shaders/unit_fragment.txt" };
		m_buffer.fill_array_buffer(unit_vertixes.data(), unit_vertixes.size() * sizeof(glm::vec2));
		m_buffer.set_vertex_attrib_pointers("2");
	}

	void prepare()
	{
		m_shader.use_program();
		m_shader.set_uniform("view", g_view_matrix);
		m_shader.set_uniform("projection", g_projection_matrix);
		m_shader.set_uniform("point_color", glm::vec3{ 0.0f, 1.0f, 0.0f });

		m_buffer.bind_vao();
	}

	void draw(glm::mat4 const& model)
	{
		m_shader.set_uniform("model", model);
		glDrawArrays(GL_POINTS, 0, m_vertices.size());
	}
};

struct arrow
{
	enum class arrow_color
	{	
		red,
		green,
		blue
	};

	std::array<float, 8> const m_vertices{-0.2f, 0.8f,
										   0.0f, 1.0f,
										   0.0f, 1.0f,
										   0.2f, 0.8f};
	shader m_shader;
	vertex_buffer m_buffer;
	arrow_color m_color;

	arrow(arrow_color color)
	:m_shader{"./shaders/line_vertex.txt", "./shaders/line_fragment.txt"},
	m_color{color}
	{
		m_buffer.fill_array_buffer(m_vertices.data(), m_vertices.size() * sizeof(float));
		m_buffer.set_vertex_attrib_pointers("2");
	}

	void prepare()
	{
		m_shader.use_program();
		m_shader.set_uniform("view", g_view_matrix);
		m_shader.set_uniform("projection", g_projection_matrix);
		glm::vec3 c{};
		switch(m_color)
		{
			case arrow_color::red:
				c = glm::vec3{1.0f, 0.0f, 0.0f};
			break;
			case arrow_color::green:
				c = glm::vec3{0.0f, 1.0f, 0.0f};
			break;
			case arrow_color::blue:
				c = glm::vec3{0.0f, 0.0f, 1.0f};
			break;
		}
		m_shader.set_uniform("line_color", c);
		m_buffer.bind_vao();
	}

	void draw(glm::mat4 const& model)
	{
		m_shader.set_uniform("model", model);
		glDrawArrays(GL_LINES, 0, 4);
	}
};

int main()
{
	srand(time(NULL));

	GLFWwindow* window = init();

	set_window_title(window);

	background_texture background_tex;
	green_circle unit;
	arrow velocity_arrow{arrow::arrow_color::green};
	arrow steering_linear_arror{arrow::arrow_color::red};
	arrow user_arrow{arrow::arrow_color::blue};

	// Units
	unsigned army_size = 1;
	unsigned army_id = create_army(army_size);
	set_formation(army_id, formation::line_along_x_towards_y, glm::vec2{0.0f, 0.0f}, 2.0f);

	double bt{};
	while (!glfwWindowShouldClose(window))
	{
		double ct = glfwGetTime();
		double const dt = ct - bt;
		bt = ct;
		
		process_input(window);

		g_view_matrix = g_cam.calc_view_matrix();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);					
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// #################################################################
		// ################### MAIN DRAWING PART ###########################
		// #################################################################

		background_tex.prepare_and_draw();

		switch(g_ai_mode)
		{
		case ai_mode::kinematic_seek:
			kinematic_seek(army_id, g_mouse_world_pos);
		break;
		case ai_mode::kinematic_flee:
			kinematic_flee(army_id, g_mouse_world_pos);
		break;
		case ai_mode::kinematic_wander:
			kinematic_wander(army_id, g_mouse_world_pos);
		break;
		case ai_mode::kinematic_arrive:
			kinematic_arrive(army_id, g_mouse_world_pos);
		break;
		case ai_mode::dynamic_seek:
			dynamic_seek(army_id, g_mouse_world_pos);
		break;
		case ai_mode::dynamic_flee:
			dynamic_flee(army_id, g_mouse_world_pos);
		break;
		case ai_mode::dynamic_arrive:
			dynamic_arrive(army_id, g_target_position);
			align(army_id, g_target_orientation);
		break;
		}

		update_army(army_id, 2.0f * dt);

		auto const army_position = get_position(army_id);
		auto const army_orientation = get_orientation(army_id);
		unit.prepare();
		for(unsigned i=0;i<army_size;++i)
		{
			glm::mat4 model{1.0f};
			model = glm::translate(model, glm::vec3{army_position[i].x, army_position[i].y, 0.0f});
			model = glm::rotate(model, army_orientation[i] - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
			unit.draw(model);
		}

		auto const army_velocity = get_velocity(army_id);
		velocity_arrow.prepare();
		for(unsigned i=0;i<army_size;++i)
		{
			glm::mat4 model{1.0f};
			model = glm::translate(model, glm::vec3{army_position[i].x, army_position[i].y, 0.0f});
			model = glm::rotate(model, atan2(army_velocity[i].y, army_velocity[i].x) - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
			velocity_arrow.draw(model);
		}

		auto const army_steering_linear = get_steering_linear(army_id);
		steering_linear_arror.prepare();
		for(unsigned i=0;i<army_size;++i)
		{
			glm::mat4 model{1.0f};
			model = glm::translate(model, glm::vec3{army_position[i].x, army_position[i].y, 0.0f});
			model = glm::rotate(model, atan2(army_steering_linear[i].y, army_steering_linear[i].x) - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
			steering_linear_arror.draw(model);
		}

		user_arrow.prepare();
		glm::mat4 model{1.0f};
		model = glm::translate(model, glm::vec3{g_target_position, 0.1f});
		model = glm::rotate(model, atan2(g_target_orientation.y, g_target_orientation.x) - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
		user_arrow.draw(model);

		// #################################################################
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}