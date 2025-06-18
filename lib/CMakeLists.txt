# Main CircularBuffer library
add_library(CircularBuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/SharedMemoryRegion.cpp
    ${CMAKE_SOURCE_DIR}/src/CircularBuffer/IWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/CircularBuffer/Writer.cpp
    ${CMAKE_SOURCE_DIR}/src/CircularBuffer/Reader.cpp
)
target_include_directories(CircularBuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)
