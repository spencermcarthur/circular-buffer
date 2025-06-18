# Main CircularBuffer library
add_library(CircularBuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/SharedMemory.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/IWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Writer.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Reader.cpp
)
target_include_directories(CircularBuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)
