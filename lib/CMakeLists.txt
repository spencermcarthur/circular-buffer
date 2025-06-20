# Main library
file(GLOB SPDLOG_SRC ${CMAKE_SOURCE_DIR}/src/spdlog/*.cpp)
add_library(circularbuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/SemaphoreLock.cpp
    ${CMAKE_SOURCE_DIR}/src/SharedMemory.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/IWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Writer.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Reader.cpp
    ${SPDLOG_SRC}
)
target_include_directories(circularbuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)
