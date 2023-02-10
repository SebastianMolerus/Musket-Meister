#include <mov.h>

#include <array>
#include <iostream>

namespace 
{
	constexpr unsigned MEM_SIZE = 4096;
	alignas(4) char MEMORY[MEM_SIZE];
	char* MEMORY_PTR{ MEMORY };

    void* allocate(size_t bytes)
    {
	    if ((MEMORY_PTR + bytes) > &MEMORY[MEM_SIZE - 1]) assert(false);
	    void* p = MEMORY_PTR;
	    MEMORY_PTR += bytes;
	    return p;
    }

    struct kinematic_data
	{
		glm::vec2* m_position;
		glm::vec2* m_velocity;
		float* m_orientation;
		float* m_rotation;
	};

    struct kinematic_steering
    {
        glm::vec2* m_linear;
        float* m_angular;
    };

    constexpr unsigned ARMIES_MAX_SIZE = 10;

    struct army
    {
        static unsigned m_army_id;
        std::array<kinematic_data, ARMIES_MAX_SIZE> m_data;
        std::array<kinematic_steering, ARMIES_MAX_SIZE> m_steering;
    } ARMIES;

    unsigned army::m_army_id{0};

    #define ARMY_EXIST(army_id) assert(army_id<=army::m_army_id && army_id >= 0)

    struct army_info
    {
        unsigned m_army_size;
    } ARMY_INFO[ARMIES_MAX_SIZE]; 

    float* get_rotation(unsigned army_id)
    {
         ARMY_EXIST(army_id);
        return ARMIES.m_data[army_id].m_rotation;
    }

    float* get_steering_angular(unsigned army_id)
    {
        ARMY_EXIST(army_id);
        return ARMIES.m_steering[army_id].m_angular;
    }

    // returns angle(rad) between vector and x axis
    float x_vector_angle_rad(float const current, glm::vec2 const vector)
    {
        if(glm::length(vector) > 0)
        {
            return atan2(vector.y, vector.x);
        }
        return current;
    }

    glm::vec2 convert_to_vec2(float const angle_rad)
    {
        return {glm::cos(angle_rad), glm::sin(angle_rad)};
    }

    constexpr float PI = 3.1415f;

    // maps rotation to -pi, pi interval.
    float map_to_range(float rotation)
    {
        if(rotation > -PI && rotation < PI) return rotation;

        while(rotation > PI)
            rotation -= 2 * PI;

        while(rotation < -PI)
            rotation += 2 * PI;

        return rotation;
    }
} // Anonymous NS

unsigned create_army(unsigned size)
{
    auto new_army_id = army::m_army_id;

    ARMIES.m_data[new_army_id].m_position = (glm::vec2*)allocate(size * sizeof(glm::vec2));
    ARMIES.m_data[new_army_id].m_velocity = (glm::vec2*)allocate(size * sizeof(glm::vec2));
    ARMIES.m_data[new_army_id].m_orientation = (float*)allocate(size * sizeof(float));
    ARMIES.m_data[new_army_id].m_rotation = (float*)allocate(size * sizeof(float));

    ARMIES.m_steering[new_army_id].m_linear = (glm::vec2*)allocate(size * sizeof(glm::vec2));
    ARMIES.m_steering[new_army_id].m_angular = (float*)allocate(size * sizeof(float));

    ARMY_INFO[new_army_id].m_army_size = size;

    army::m_army_id++;
    return new_army_id;
}

void set_formation(unsigned army_id, formation f, glm::vec2 spawn, float spacing)
{
    ARMY_EXIST(army_id);

    auto const army_size = ARMY_INFO[army_id].m_army_size;
    auto& kin_data = ARMIES.m_data[army_id];

    switch(f)
    {
        case formation::line_along_x_towards_y:
            glm::vec2 s = spawn;
            for(unsigned i=0;i<army_size;++i)
            {
                kin_data.m_position[i] = s;
                s.x += spacing;
            }
        break;
    }
}

void update_army(unsigned army_id, float dt)
{
    ARMY_EXIST(army_id);

    auto v = get_velocity(army_id);
    auto p = get_position(army_id);
    auto o = get_orientation(army_id);
    auto r = get_rotation(army_id);
    auto const sl = get_steering_linear(army_id);
    auto const sa = get_steering_angular(army_id);

    auto const army_size = ARMY_INFO[army_id].m_army_size;

    constexpr float max_velocity = 3.0f;

    for(unsigned i=0;i<army_size;++i)
    {
        p[i] += v[i] * dt;
        o[i] += r[i] * dt;

        v[i] += sl[i] * dt;
        r[i] += sa[i] * dt;

        // this works for dynamic only
        // because kinematic version updated position
        // at this point
        if(glm::length(v[i]) > max_velocity)
        {
            v[i] = glm::normalize(v[i]) * max_velocity;
        }
    }
}

glm::vec2* get_position(unsigned army_id)
{
    ARMY_EXIST(army_id);
    return ARMIES.m_data[army_id].m_position;
}

float* get_orientation(unsigned army_id)
{
    ARMY_EXIST(army_id);
    return ARMIES.m_data[army_id].m_orientation;
}

glm::vec2* get_steering_linear(unsigned army_id)
{
    ARMY_EXIST(army_id);
    return ARMIES.m_steering[army_id].m_linear;
}

glm::vec2* get_velocity(unsigned army_id)
{
    ARMY_EXIST(army_id);
    return ARMIES.m_data[army_id].m_velocity;
}

void kinematic_seek(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    auto v = get_velocity(army_id);
    auto o = get_orientation(army_id);

    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = target_pos - p[i];
        v[i] = glm::normalize(v[i]);

        o[i] = x_vector_angle_rad(o[i], v[i]);
    }
}

void kinematic_flee(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    auto v = get_velocity(army_id);
    auto o = get_orientation(army_id);

    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = p[i] - target_pos;
        v[i] = glm::normalize(v[i]);

        o[i] = x_vector_angle_rad(o[i], v[i]);
    }
}

void kinematic_arrive(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    constexpr float max_speed{3.0f};
    constexpr float velocity_boost{4.0f};

    auto v = get_velocity(army_id);

    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = target_pos - p[i];

        v[i] *= velocity_boost;

        if(glm::length(v[i]) > max_speed)
        {
            v[i] = glm::normalize(v[i]) * max_speed;
        }
    }
}
void kinematic_wander(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    auto v = get_velocity(army_id);
    auto r = get_rotation(army_id);

    auto const o = get_orientation(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = convert_to_vec2(o[i]);

        r[i] = (rand()%2) - (rand()%2);
    }
}

void dynamic_seek(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    constexpr float max_acc = 4.0f;

    auto sl = get_steering_linear(army_id);

    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        sl[i] = glm::normalize(target_pos - p[i]);
        sl[i] *= max_acc;
    }
}

void dynamic_flee(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    constexpr float max_acc = 4.0f;

    auto sl = get_steering_linear(army_id);

    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        sl[i] = glm::normalize(p[i] - target_pos);
        sl[i] *= max_acc;
    }
}

void dynamic_arrive(unsigned army_id, glm::vec2 target_pos)
{
    ARMY_EXIST(army_id);

    constexpr float slow_radius{2.0f};
    constexpr float max_speed{3.0f};
    constexpr float max_acceleration{3.0f};
    constexpr float acceleration_boost{10.0f};

    auto sl = get_steering_linear(army_id);

    auto const v = get_velocity(army_id);
    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        auto const dir = target_pos - p[i];
        float const distance = glm::length(dir);

        // closer to target, lower speed
        float target_speed{};
        if(distance > slow_radius)
        {
            target_speed = max_speed;
        }
        else
        {
            target_speed = max_speed * distance / slow_radius;
        }
        auto const goal_velocity = glm::normalize(dir) * target_speed;

        // sl will be opposite do v
        // when we are close to the target
        // but this value will be too small
        // to lose that speed and we will start wiggling
        // so...
        sl[i] = goal_velocity - v[i];

        // ...we need to increase acceleration
        sl[i] *= acceleration_boost;

        if(glm::length(sl[i]) > max_acceleration)
        {
            sl[i] = glm::normalize(sl[i]) * max_acceleration;
        }
    }
}

void align(unsigned army_id, glm::vec2 target_orientation)
{
    ARMY_EXIST(army_id);

    constexpr float slow_radius{2.0f};
    constexpr float max_rotation{3.0f};
    constexpr float max_angular_steering{3.0f};

    float* steering_angular = get_steering_angular(army_id);

    float* const o = get_orientation(army_id);
    float* const r = get_rotation(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;
    for(unsigned i=0;i<army_size;++i)
    {
        float goal_rotation =  x_vector_angle_rad(0.0f, target_orientation) - o[i];

        //  -PI ... +PI
        goal_rotation = map_to_range(goal_rotation);
        float const rotation_size = glm::abs(goal_rotation);

        float target_rotation{};
        if(rotation_size > slow_radius )
            target_rotation = max_rotation;
        else
        {
            target_rotation = max_rotation * rotation_size / slow_radius;
        }

        target_rotation *= glm::sign(goal_rotation);

        steering_angular[i] = target_rotation - r[i];
        steering_angular[i] *= 10.0f; // same reason as for dynamic arrive

        auto const steering_value = glm::abs(steering_angular[i]);
        if(steering_value > max_angular_steering)
        {
            steering_angular[i] = glm::sign(steering_angular[i]) * max_angular_steering; 
        }
    }
}