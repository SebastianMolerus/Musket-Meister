cmake_minimum_required(VERSION 3.2)

# Project Version -> Major.Minor.Patch.Tweak
# Major - breaking change
# Minor - increment
# Patch - cleaning
# Tweak - bug fix

project(musket-meister
        VERSION 0.6.2.0
        LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_compile_definitions($<$<BOOL:$<CONFIG:Debug>>:"__DEBUG__">)

find_package(OpenGL REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_subdirectory(src)