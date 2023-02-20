#include <iostream>
#include <array>
#include <vector>
#include <utility>

#include <mov.h>
#include <utils.h>
#include <stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

const int g_window_width = 1920;
const int g_window_height = 1080;

camera g_cam{g_window_width, g_window_height};

bool g_LMB_pressed{};

glm::mat4 g_view_matrix{1.0f};

glm::vec3 g_mouse_world_pos{1.0f};

glm::mat4 const g_projection_matrix{g_cam.get_projection_matrix()};

glm::vec2 g_target_position{};
glm::vec2 g_target_orientation{};

glm::vec2 g_previous_mouse_pos{0.0f, 0.0f};

std::vector<std::pair<glm::vec2, float>> g_path;

glm::vec2 pp[50]{};

enum class color
{
    red,
    green,
    blue,
    orange
};

glm::vec3 get_color(color c)
{
    switch (c)
    {
    case color::red:
        return glm::vec3{1.0f, 0.0f, 0.0f};
        break;
    case color::green:
        return glm::vec3{0.0f, 1.0f, 0.0f};
        break;
    case color::blue:
        return glm::vec3{0.0f, 0.0f, 1.0f};
        break;
    case color::orange:
        return glm::vec3{0.9f, 0.5f, 0.1f};
        break;
    }
}

enum class ai_mode
{
    kinematic_seek,
    kinematic_flee,
    kinematic_wander,
    kinematic_arrive,

    dynamic_seek,
    dynamic_flee,
    dynamic_arrive,

    pursue,

    velocity_match,

    wander,

    path_follow
} g_ai_mode;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    double const x_ndc{(xpos / g_window_width) * 2.0f - 1};
    double const y_ndc{-((ypos / g_window_height) * 2.0f - 1)};

    glm::vec3 const plane_normal{0.0, 0.0f, 1.0f};
    glm::vec4 xy_view = glm::inverse(g_projection_matrix) * glm::vec4{x_ndc, y_ndc, 0.0f, 0.0f};
    xy_view.z = -1;
    glm::vec3 const ray_world = glm::normalize(glm::inverse(g_view_matrix) * xy_view);
    auto const cam_pos = g_cam.get_camera_pos();
    const float t = (-glm::dot(plane_normal, cam_pos)) / (glm::dot(plane_normal, ray_world));
    g_mouse_world_pos = cam_pos + t * ray_world;

    if (g_LMB_pressed)
    {
        g_target_orientation = glm::normalize(glm::vec2{g_mouse_world_pos.x, g_mouse_world_pos.y} - g_target_position);
    }

    if(g_ai_mode != ai_mode::path_follow)
    {
        auto const dir = g_mouse_world_pos - glm::vec3{g_previous_mouse_pos, 0.0f};
        if(glm::length(dir) > 3.0f)
        {
            g_path.push_back({g_mouse_world_pos, atan2(dir.y, dir.x)});
            g_previous_mouse_pos = g_mouse_world_pos;
        }
    }
}

void mouse_click_callback(GLFWwindow *, int button, int action, int)
{
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS)
    {
        g_target_position = g_mouse_world_pos;
        g_LMB_pressed = true;
    }

    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE)
    {
        g_LMB_pressed = false;
    }
}

void keyboard_key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case GLFW_KEY_O:
        g_ai_mode = ai_mode::dynamic_arrive;
        break;
    case GLFW_KEY_P:
        g_ai_mode = ai_mode::pursue;
        break;
    case GLFW_KEY_Q:
        g_ai_mode = ai_mode::wander;
    }
}

GLFWwindow *init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(g_window_width, g_window_height, "Musket-Meister", NULL, NULL);
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

void process_input(GLFWwindow *window)
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

void set_window_title(GLFWwindow *window)
{
    std::string window_title{"Musket-Meister. v:"};
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
            2, 3, 1};

    // 200 x 200
    const std::array<float, 20> m_vertices =
    {
            -100.0f, -100.0f, -1.0f, 0.0f, 0.0f,  // bottom left
            100.0f, -100.0f, -1.0f, 200.0f, 0.0f, // bottom right
            -100.0f, 100.0f, -1.0f, 0.0f, 200.0f, // top left
            100.0f, 100.0f, -1.0f, 200.0f, 200.0f // top right
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
        m_shader.set_uniform("model", glm::mat4{1.0f});
        m_shader.set_uniform("projection", g_projection_matrix);

        m_buffer.bind_vao();
        glBindTexture(GL_TEXTURE_2D, m_texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
};

struct circle
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
        for (unsigned i = 0; i < res.size() / 2; ++i)
        {
            res[i].x = x;

            if (x < 0.0f)
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
    color m_color;

    circle(color color)
    :m_vertices{direction_circle()},
    m_shader{"./shaders/unit_vertex.txt", "./shaders/unit_fragment.txt"},
    m_color{color}
    {
        auto unit_vertixes = direction_circle();
        shader unit_shader{"./shaders/unit_vertex.txt", "./shaders/unit_fragment.txt"};
        m_buffer.fill_array_buffer(unit_vertixes.data(), unit_vertixes.size() * sizeof(glm::vec2));
        m_buffer.set_vertex_attrib_pointers("2");
    }

    void prepare()
    {
        m_shader.use_program();
        m_shader.set_uniform("view", g_view_matrix);
        m_shader.set_uniform("projection", g_projection_matrix);
        m_shader.set_uniform("point_color", get_color(m_color));
        m_buffer.bind_vao();
    }

    void draw(glm::mat4 const &model)
    {
        m_shader.set_uniform("model", model);
        glDrawArrays(GL_POINTS, 0, m_vertices.size());
    }
};

struct arrow
{
    std::array<float, 12> const m_vertices{-0.2f, 0.8f,
                                           0.0f, 1.0f,
                                           0.0f, 1.0f,
                                           0.2f, 0.8f,
                                           0.0f, 1.0f,
                                           0.0f, 0.0f};
    color m_color;
    shader m_shader;
    vertex_buffer m_buffer;
    arrow(color color)
    :m_color{color},
    m_shader{"./shaders/line_vertex.txt", "./shaders/line_fragment.txt"}
    {
        m_buffer.fill_array_buffer(m_vertices.data(), m_vertices.size() * sizeof(float));
        m_buffer.set_vertex_attrib_pointers("2");
    }

    void prepare()
    {
        m_shader.use_program();
        m_shader.set_uniform("view", g_view_matrix);
        m_shader.set_uniform("projection", g_projection_matrix);
        m_shader.set_uniform("line_color", get_color(m_color));
        m_buffer.bind_vao();
    }

    void draw(glm::mat4 const &model)
    {
        m_shader.set_uniform("model", model);
        glDrawArrays(GL_LINES, 0, 6);
    }
};

struct army
{
    unsigned m_id;
    unsigned m_size;
    explicit army(unsigned size)
    :m_size{size}
    {
        m_id = create_army(m_size);
    }

    glm::vec2 *vel() const { return get_velocity(m_id); }
    glm::vec2 *pos() const { return get_position(m_id); }

    operator unsigned() const { return m_id; }

    unsigned size() const { return m_size; }
};

int main()
{
    srand(time(NULL));

    GLFWwindow *window = init();

    set_window_title(window);

    background_texture background_tex;

    circle green_circle{color::green};
    circle red_circle{color::red};

    arrow velocity_arrow{color::green};
    arrow red_steering_linear_arrow{color::red};
    arrow user_arrow{color::blue};
    arrow path_arrow{color::orange};

    // Units
    army green_army(1);
    army red_army(1);

    double bt{};
    while (!glfwWindowShouldClose(window))
    {
        double ct = glfwGetTime();
        double const dt = ct - bt;
        std::cout<<dt<<std::endl;
        bt = ct;

        process_input(window);

        g_view_matrix = g_cam.calc_view_matrix();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // #################################################################
        // ################### MAIN DRAWING PART ###########################
        // #################################################################

        background_tex.prepare_and_draw();

        switch (g_ai_mode)
        {
        case ai_mode::kinematic_seek:
            kinematic_seek(green_army, g_mouse_world_pos);
            break;
        case ai_mode::kinematic_flee:
            kinematic_flee(green_army, g_mouse_world_pos);
            break;
        case ai_mode::kinematic_wander:
            kinematic_wander(green_army, g_mouse_world_pos);
            break;
        case ai_mode::kinematic_arrive:
            kinematic_arrive(green_army, g_mouse_world_pos);
            break;
        case ai_mode::dynamic_seek:
            dynamic_seek(green_army, g_mouse_world_pos);
            break;
        case ai_mode::dynamic_flee:
            dynamic_flee(green_army, g_mouse_world_pos);
            break;
        case ai_mode::dynamic_arrive:
            dynamic_arrive(green_army, g_target_position);
            align(green_army, g_target_orientation);
            break;
        case ai_mode::velocity_match:
            // Green army will match red army velocity
            dyn_velocity_match(green_army, red_army.vel()[0]);
            break;
        case ai_mode::pursue:
            // Green army will pursue red army
            pursue(green_army, red_army.pos()[0], red_army.vel()[0]);
            face(green_army, red_army.pos()[0]);
            break;
        case ai_mode::wander:
            wander(green_army);
            break;
        case ai_mode::path_follow:
            break;
        }

        // red army follow mouse on screen
        dynamic_arrive(red_army, g_mouse_world_pos);
        look_where_you_going(red_army);

        update_army(green_army, 2.0f * dt);
        update_army(red_army, 2.0f * dt);

        glm::mat4 const *ra_models_p_o = calculate_models_p_o(red_army);
        red_circle.prepare();
        for (unsigned i = 0; i < red_army.size(); ++i)
        {
            red_circle.draw(ra_models_p_o[i]);
        }

        glm::mat4 const *ga_models_p_o = calculate_models_p_o(green_army);
        green_circle.prepare();
        for (unsigned i = 0; i < green_army.size(); ++i)
        {
            green_circle.draw(ga_models_p_o[i]);
        }

        glm::mat4 const *ga_models_p_v = calculate_models_p_v(green_army);
        velocity_arrow.prepare();
        for (unsigned i = 0; i < green_army.size(); ++i)
        {
            velocity_arrow.draw(ga_models_p_v[i]);
        }

        red_steering_linear_arrow.prepare();
        glm::mat4 const *ga_models_p_sl = calculate_models_p_sl(green_army);
        for (unsigned i = 0; i < green_army.size(); ++i)
        {
            red_steering_linear_arrow.draw(ga_models_p_sl[i]);
        }

        user_arrow.prepare();
        glm::mat4 model{1.0f};
        model = glm::translate(model, glm::vec3{g_target_position, 0.1f});
        model = glm::rotate(model, atan2(g_target_orientation.y, g_target_orientation.x) - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
        user_arrow.draw(model);

        path_arrow.prepare();
        for(auto& [pos, rot] : g_path)
        {
            glm::mat4 model{1.0f};
            model = glm::translate(model, glm::vec3{pos, 0.0f});
            model = glm::rotate(model, rot - glm::radians(90.0f), glm::vec3{0.0f, 0.0f, 1.0f});
            path_arrow.draw(model);
        }

        // #################################################################

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}