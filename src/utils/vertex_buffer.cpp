#include <utils.h>

vertex_buffer::vertex_buffer()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);	
}
	
void vertex_buffer::fill_array_buffer(void const* const buffer, unsigned size)
{
    unsigned vbo{};
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
}

void vertex_buffer::fill_element_array_buffer(void const* const buffer, unsigned size)
{
    unsigned ebo{};
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);
}

void vertex_buffer::bind_vao()
{
    glBindVertexArray(m_vao);
}

void vertex_buffer::set_vertex_attrib_pointers(const char* pattern)
{
    unsigned stride{};
    const char* cc = pattern;
    while (*cc)
    {
        stride += *cc - '0';
        ++cc;
    }

    unsigned index{};
    unsigned offset{};
    for (const char* pc = pattern; *pc != 0; ++pc)
    {
        const unsigned number = *pc - '0';

        glVertexAttribPointer(index, number, GL_FLOAT, GL_FALSE,
            stride * sizeof(float), (void*)(offset * sizeof(float)));
        glEnableVertexAttribArray(index);
        ++index;
        offset += number;
    }
}
