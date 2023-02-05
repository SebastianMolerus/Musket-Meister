cmake_minimum_required(VERSION 3.2)

add_executable(main main.cpp)

target_compile_definitions(main
    PRIVATE
    __PROJECT_VERSION__="${CMAKE_PROJECT_VERSION}")

target_link_libraries(main
    PRIVATE
    utils)

add_subdirectory(utils)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR}/src)
file(COPY ${CMAKE_CURRENT_LIST_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/src)