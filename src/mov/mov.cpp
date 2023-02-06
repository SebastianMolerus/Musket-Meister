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

    glm::vec2* get_velocity(unsigned army_id)
    {
        ARMY_EXIST(army_id);
        return ARMIES.m_data[army_id].m_velocity;
    }

    float* get_rotation(unsigned army_id)
    {
         ARMY_EXIST(army_id);
        return ARMIES.m_data[army_id].m_rotation;
    }

    glm::vec2* get_steering_linear(unsigned army_id)
    {
        ARMY_EXIST(army_id);
        return ARMIES.m_steering[army_id].m_linear;
    }

    float* get_steering_angular(unsigned army_id)
    {
        ARMY_EXIST(army_id);
        return ARMIES.m_steering[army_id].m_angular;
    }

    // returns angle between vector and x axis
    float x_target_angle_rad(float current, glm::vec2 target)
    {
        if(target.length() > 0)
        {
            return atan2(target.y, target.x);
        }
        return current;
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

    for(unsigned i=0;i<army_size;++i)
    {
        p[i] += v[i] * dt;
        o[i] += r[i] * dt;

        v[i] += sl[i] * dt;
        r[i] += sa[i] * dt;
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

void kinematic_seek(unsigned army_id, glm::vec2 target)
{
    ARMY_EXIST(army_id);
    auto v = get_velocity(army_id);
    auto o = get_orientation(army_id);
    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = target - p[i];
        v[i] = glm::normalize(v[i]);

        o[i] = x_target_angle_rad(o[i], v[i]);
    }
}

void kinematic_flee(unsigned army_id, glm::vec2 target)
{
    ARMY_EXIST(army_id);
    auto v = get_velocity(army_id);
    auto o = get_orientation(army_id);
    auto const p = get_position(army_id);
    auto const army_size = ARMY_INFO[army_id].m_army_size;

    for(unsigned i=0;i<army_size;++i)
    {
        v[i] = p[i] - target;
        v[i] = glm::normalize(v[i]);

        o[i] = x_target_angle_rad(o[i], v[i]);
    }
}