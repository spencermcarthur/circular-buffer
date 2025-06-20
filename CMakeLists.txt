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
    CB_MAX_SHARED_MEM_SIZE_MIB=${MAX_SHARED_MEM_SIZE_MIB}
    CB_MAX_MESSAGE_SIZE_BYTES=${MAX_MESSAGE_SIZE_BYTES}
    CB_CACHELINE_SIZE_BYTES=${CACHELINE_SIZE_BYTES}
    SPDLOG_COMPILED_LIB=1  # Compile as shared library
    SPDLOG_USE_STD_FORMAT=1  # Use <format> header instead of libfmt
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
    add_compile_definitions(DEBUG=1 SPDLOG_ACTIVE_LEVEL=1)
endif()


# Subdirs
add_subdirectory(bin)
add_subdirectory(lib)
add_subdirectory(tests)


# Install
install(TARGETS circularbuffer)
install(DIRECTORY include DESTINATION ${CMAKE_INSTALL_PREFIX} PATTERN include/spdlog EXCLUDE)
