#include <iostream>
#include <array>

#include <utils.h>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

const int g_window_width = 1920;
const int g_window_height = 1080;

camera g_cam{g_window_width, g_window_height};

glm::mat4 g_view_matrix{1.0f};

glm::vec3 g_mouse_world_pos{1.0f};

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	double const x_ndc{ (xpos / g_window_width) * 2.0f - 1 };
	double const y_ndc{ -((ypos / g_window_height) * 2.0f - 1) };

	glm::vec3 const plane_normal{ 0.0, 0.0f, 1.0f };
	glm::vec4 xy_view = glm::inverse(g_cam.get_projection_matrix()) * glm::vec4{ x_ndc, y_ndc, 0.0f, 0.0f };
	xy_view.z = -1;
	glm::vec3 const ray_world = glm::normalize(glm::inverse(g_view_matrix) * xy_view);
	auto const cam_pos = g_cam.get_camera_pos();
	const float t = (-glm::dot(plane_normal, cam_pos)) / (glm::dot(plane_normal, ray_world));
	g_mouse_world_pos = cam_pos + t * ray_world;
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


	stbi_set_flip_vertically_on_load(true);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD\n";
		glfwTerminate();
	}

	glEnable(GL_PROGRAM_POINT_SIZE); 

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

const std::array<glm::vec2, 200> direction_circle()
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

// 200 x 200
const std::array<float, 20> grass_vertices =
{
	-100.0f, -100.0f, 0.0f, 0.0f, 0.0f, // bottom left
	100.0f, -100.0f, 0.0f, 200.0f, 0.0f, // bottom right
	-100.0f, 100.0f, 0.0f, 0.0f, 200.0f, // top left
	100.0f, 100.0f, 0.0f, 200.0f, 200.0f  // top right
};

const std::array<int, 6> grass_indices = 
{
	0, 1, 2,
	2, 3, 1
};

int main()
{
	GLFWwindow* window = init();

	// Prepare main background texture
	shader grass_shader{ "./shaders/grass_vertex.txt", "./shaders/grass_fragment.txt" };
	unsigned grass_texture = load_texture("./textures/grass.jpg");
	vertex_buffer grass_buff;
	grass_buff.fill_array_buffer(grass_vertices.data(), grass_vertices.size() * sizeof(float));
	grass_buff.fill_element_array_buffer(grass_indices.data(), grass_indices.size() * sizeof(int));
	grass_buff.set_vertex_attrib_pointers("32");

	// Prepare unit "texture"
	auto unit_vertixes = direction_circle();
	shader unit_shader{ "./shaders/unit_vertex.txt", "./shaders/unit_fragment.txt" };
	vertex_buffer unit_buff;
	unit_buff.fill_array_buffer(unit_vertixes.data(), unit_vertixes.size() * sizeof(glm::vec2));
	unit_buff.set_vertex_attrib_pointers("2");

	std::string window_title {"Musket-Meister. v:"};
	{
		#ifdef __PROJECT_VERSION__
		window_title += __PROJECT_VERSION__;
		glfwSetWindowTitle(window, window_title.c_str());
		#endif
	}

	while (!glfwWindowShouldClose(window))
	{
		process_input(window);

		g_view_matrix = g_cam.calc_view_matrix();
		auto const projection = g_cam.get_projection_matrix();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);					
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		{	
			grass_shader.use_program();
			grass_shader.set_uniform("view", g_view_matrix);
			grass_shader.set_uniform("model", glm::mat4{ 1.0f });
			grass_shader.set_uniform("projection", projection);
			grass_buff.bind_vao();
			glBindTexture(GL_TEXTURE_2D, grass_texture);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		{
			auto model = glm::mat4{1.0f};
			model = glm::translate(model, g_mouse_world_pos);
			unit_shader.use_program();
			unit_shader.set_uniform("view", g_view_matrix);
			unit_shader.set_uniform("model", model);
			unit_shader.set_uniform("projection", projection);
			unit_shader.set_uniform("point_color", glm::vec3{ 0.0f, 1.0f, 0.0f });
			unit_buff.bind_vao();
			glDrawArrays(GL_POINTS, 0, unit_vertixes.size());
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}