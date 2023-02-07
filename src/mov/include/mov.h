#ifndef MOV_H
#define MOV_H

#include <glm/glm.hpp>

enum class formation
{
    line_along_x_towards_y
};

unsigned create_army(unsigned size);

void set_formation(unsigned army_id, formation f, glm::vec2 spawn, float spacing = 1.0f);

void update_army(unsigned army_id, float dt);

glm::vec2* get_position(unsigned army_id);
float* get_orientation(unsigned army_id);
glm::vec2* get_steering_linear(unsigned army_id);
glm::vec2* get_velocity(unsigned army_id);

void kinematic_seek(unsigned army_id, glm::vec2 target);
void kinematic_flee(unsigned army_id, glm::vec2 target);
void kinematic_wander(unsigned army_id, glm::vec2 target);

#endif