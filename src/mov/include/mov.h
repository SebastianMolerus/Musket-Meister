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

// Those are calculating velocities directly
void kinematic_seek(unsigned army_id, glm::vec2 target_pos);
void kinematic_flee(unsigned army_id, glm::vec2 target_pos);
void kinematic_arrive(unsigned army_id, glm::vec2 target_pos);
void kinematic_wander(unsigned army_id, glm::vec2 target_pos);

// Those are calculating velocities indirectly by using steerings
void dynamic_seek(unsigned army_id, glm::vec2 target_pos);
void dynamic_flee(unsigned army_id, glm::vec2 target_pos);
void dynamic_arrive(unsigned army_id, glm::vec2 target_pos);

void pursue(unsigned army_id, glm::vec2 target_pos, glm::vec2 target_velocity);

// copy target orientation
void align(unsigned army_id, glm::vec2 target_orientation);

// face at target
void face(unsigned army_id, glm::vec2 target_pos);

// copy target velocity
void dyn_velocity_match(unsigned army_id, glm::vec2 target_velocity);

// set orientation from own velocity
void look_where_you_going(unsigned army_id);

#endif