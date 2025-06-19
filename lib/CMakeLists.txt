file(GLOB SPDLOG_SRC ${CMAKE_SOURCE_DIR}/src/spdlog/*.cpp)
add_library(spdlog SHARED
    ${SPDLOG_SRC}
)
target_include_directories(spdlog PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(spdlog PUBLIC 
    SPDLOG_COMPILED_LIB=1
    SPDLOG_USE_STD_FORMAT=1
)


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
