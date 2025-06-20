include_directories(${CMAKE_SOURCE_DIR}/include)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    link_libraries(asan circularbuffer)
else()
    link_libraries(circularbuffer)
endif()

add_executable(WriterApp EXCLUDE_FROM_ALL WriterApp.cpp)
add_executable(ReaderApp EXCLUDE_FROM_ALL ReaderApp.cpp)

add_executable(ReaderWriterApp EXCLUDE_FROM_ALL ReaderWriterApp.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/bufferconfig.txt 
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_target(Apps
    DEPENDS
        WriterApp
        ReaderApp
        ReaderWriterApp
)

add_subdirectory(benchmarks)
