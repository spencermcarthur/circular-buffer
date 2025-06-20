# Main library
file(GLOB CB_SRC ${CMAKE_SOURCE_DIR}/src/circularbuffer/*.cpp)
file(GLOB SPDLOG_SRC ${CMAKE_SOURCE_DIR}/src/spdlog/*.cpp)

add_library(circularbuffer SHARED ${CB_SRC} ${SPDLOG_SRC})

target_include_directories(circularbuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)
file(GLOB CB_HEADERS ${CMAKE_SOURCE_DIR}/include/**.hpp)
set_target_properties(circularbuffer PROPERTIES PUBLIC_HEADER "${CB_HEADERS}")
