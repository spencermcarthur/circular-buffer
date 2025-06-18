add_library(circularbuffer SHARED
    ${CMAKE_SOURCE_DIR}/src/SharedMemoryRegion.cpp)
target_include_directories(circularbuffer PUBLIC ${CMAKE_SOURCE_DIR}/include)