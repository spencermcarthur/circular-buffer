# CircularBuffer shared library
add_library(CircularBuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/Message.cpp
    ${CMAKE_SOURCE_DIR}/src/CircularBuffer.cpp
)
target_include_directories(CircularBuffer PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)