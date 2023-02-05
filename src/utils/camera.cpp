#include <utils.h>
#include <glm/gtc/matrix_transform.hpp>

camera::camera(unsigned screen_width, unsigned screen_height)
:m_projection{glm::perspective(glm::radians(45.0f), (float)screen_width / (float)screen_height, 0.1f, 1000.0f)}
{
}

void camera::move_up(float value)
{
    m_camera_pos.y += value;
}

void camera::move_down(float value)
{
    m_camera_pos.y -= value;
}

void camera::move_right(float value)
{
    m_camera_pos.x += value;
}

void camera::move_left(float value)
{
    m_camera_pos.x -= value;
}

glm::mat4 const& camera::calc_view_matrix()
{
    m_view = glm::lookAt(m_camera_pos, m_camera_pos + m_camera_direction, m_up);
    return m_view;
}

glm::mat4 const& camera::get_projection_matrix() const
{
    return m_projection;
}

glm::vec3 const& camera::get_camera_pos() const
{
    return m_camera_pos;
}