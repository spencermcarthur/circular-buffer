cmake_minimum_required(VERSION 3.23)
project(circularbuffer)


# Ensure Linux
if(NOT LINUX)
    message(FATAL_ERROR "Written for Linux operating systems")
endif()


# Ensure C++ 20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)


# Set max shared memory size
if(NOT MAX_SHARED_MEM_SIZE_MIB)
    set(MAX_SHARED_MEM_SIZE_MIB 50)
    message(STATUS "MAX_SHARED_MEM_SIZE_MIB defaulting to ${MAX_SHARED_MEM_SIZE_MIB}")
endif()


# Set max message size
if(NOT MAX_MESSAGE_SIZE_BYTES)
    set(MAX_MESSAGE_SIZE_BYTES 65535)
    message(STATUS "MAX_MESSAGE_SIZE_BYTES defaulting to ${MAX_MESSAGE_SIZE_BYTES}")
endif()


# Get system cacheline size and define it (defaults to 64)
if(NOT GETCONF_CACHELINE_SIZE_VAR)
    set(GETCONF_CACHELINE_SIZE_VAR "LEVEL1_DCACHE_LINESIZE")
endif()

execute_process(COMMAND "getconf" ${GETCONF_CACHELINE_SIZE_VAR}
    OUTPUT_VARIABLE CACHELINE_SIZE_BYTES
    ERROR_VARIABLE CLSIZE_ERR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ECHO_ERROR_VARIABLE
)

if(CLSIZE_ERR)
    message(WARNING "Error getting cacheline size: will default to 64")
    set(CACHELINE_SIZE_BYTES 64)
else()
    message(STATUS "Read cacheline size from system")
endif()


# Definitions
add_compile_definitions(
    MAX_SHARED_MEM_SIZE_MIB=${MAX_SHARED_MEM_SIZE_MIB}
    MAX_MESSAGE_SIZE_BYTES=${MAX_MESSAGE_SIZE_BYTES}
    CACHELINE_SIZE_BYTES=${CACHELINE_SIZE_BYTES}
)


# Warnings
add_compile_options(-Wall -Wextra -Wpedantic)


# Options
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Building Release configuration")
    add_compile_options(-Ofast)
else()
    message(STATUS "Building Debug configuration")
    add_compile_options(-ggdb3)
endif()


# Subdirs
add_subdirectory(${CMAKE_SOURCE_DIR}/lib)
add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
