find_package(benchmark REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_libraries(circularbuffer benchmark::benchmark)

# Writer
add_executable(WriterBenchmarks EXCLUDE_FROM_ALL Writer.cpp)

add_custom_target(Benchmarks
    DEPENDS
        WriterBenchmarks
)
