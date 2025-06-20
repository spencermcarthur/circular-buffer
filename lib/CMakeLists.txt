file(GLOB SPDLOG_SRC ${CMAKE_SOURCE_DIR}/src/spdlog/*.cpp)
add_library(spdlog SHARED
    ${SPDLOG_SRC}
)
target_include_directories(spdlog PUBLIC ${CMAKE_SOURCE_DIR}/include)


# Main CircularBuffer library
add_library(circularbuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/SemaphoreLock.cpp
    ${CMAKE_SOURCE_DIR}/src/SharedMemory.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/IWrapper.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Writer.cpp
    ${CMAKE_SOURCE_DIR}/src/circularbuffer/Reader.cpp
)
target_include_directories(circularbuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(circularbuffer PUBLIC spdlog)
