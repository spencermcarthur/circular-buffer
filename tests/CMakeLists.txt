find_package(GTest REQUIRED)

add_library(testing SHARED EXCLUDE_FROM_ALL
    Utils.cpp)

add_executable(SharedMemoryRegion EXCLUDE_FROM_ALL SharedMemoryRegion.cpp)
target_link_libraries(SharedMemoryRegion PUBLIC circularbuffer testing GTest::gtest GTest::gtest_main)

add_custom_target(tests
    DEPENDS SharedMemoryRegion)
