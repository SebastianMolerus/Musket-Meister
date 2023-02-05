add_library(utils
    shader.cpp
    texture.cpp
    vertex_buffer.cpp
    stb.cpp
    camera.cpp)

target_include_directories(utils
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

target_link_libraries(utils
    PUBLIC
    glm::glm
    OpenGL::GL
    glad::glad
    glfw)
