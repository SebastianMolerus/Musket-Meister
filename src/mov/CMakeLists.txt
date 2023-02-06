add_library(mov
    mov.cpp)

target_include_directories(mov
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

target_link_libraries(mov
    PUBLIC
    glm::glm)