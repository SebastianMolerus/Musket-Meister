add_library(mov
    mov.cpp)

set_target_properties(mov
PROPERTIES
COMPILE_FLAGS
"-save-temps -masm=intel -fno-asynchronous-unwind-tables -fno-exceptions -fno-rtti -fverbose-asm") 

target_include_directories(mov
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>)

target_link_libraries(mov
    PUBLIC
    glm::glm)