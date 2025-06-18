cmake_minimum_required(VERSION 3.20)

project(circular_buffer)

if(NOT LINUX)
    message(FATAL_ERROR "Written for Linux operating systems")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Ofast)
else()
    add_compile_options(-ggdb3)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
