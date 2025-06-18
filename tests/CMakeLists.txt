enable_testing()

find_package(GTest REQUIRED)

# Testing library
add_library(testing SHARED EXCLUDE_FROM_ALL
    Utils.cpp)

############################## TESTS ##############################
# SharedMemoryRegion
add_executable(SharedMemoryRegion EXCLUDE_FROM_ALL SharedMemoryRegion.cpp)
target_link_libraries(SharedMemoryRegion PUBLIC circularbuffer testing GTest::gtest GTest::gtest_main)
add_test(NAME SharedMemoryRegion COMMAND SharedMemoryRegion)
###################################################################

# Target to make all tests
add_custom_target(all_tests
    DEPENDS SharedMemoryRegion)
