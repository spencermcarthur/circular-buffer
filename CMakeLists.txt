cmake_minimum_required(VERSION 3.23)
project(circular_buffer)


# Ensure Linux
if(NOT LINUX)
    message(FATAL_ERROR "Written for Linux operating systems")
endif()


# Ensure C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)


# Enable warnings
add_compile_options(-Wall -Wextra -Wpedantic)


# Get system cacheline size and define it (defaults to 64)
if(NOT GETCONF_CLSIZE_VAR)
    set(GETCONF_CLSIZE_VAR "LEVEL1_DCACHE_LINESIZE")
endif()
execute_process(COMMAND "getconf" ${GETCONF_CLSIZE_VAR}
    OUTPUT_VARIABLE CL_SIZE
    ERROR_VARIABLE CLSIZE_ERR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ECHO_ERROR_VARIABLE)
if(CLSIZE_ERR)
    message(WARNING "Error getting cacheline size: will default to 64")
else()
    # Define CACHELINE_SIZE_BYTES
    add_compile_definitions(CACHELINE_SIZE_BYTES=${CL_SIZE})
endif()


# Build type-specific compilation options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-Ofast)
else()
    add_compile_options(-ggdb3)
endif()


# Subdirs
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
