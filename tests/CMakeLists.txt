enable_testing()

find_package(GTest REQUIRED)

# Testing library
add_library(Testing SHARED EXCLUDE_FROM_ALL
    Utils.cpp)

link_libraries(CircularBuffer Testing GTest::gtest GTest::gtest_main)

############################## TESTS ##############################
# SharedMemoryRegion
add_executable(SharedMemoryRegion EXCLUDE_FROM_ALL SharedMemoryRegion.cpp)
add_test(NAME SharedMemoryRegion COMMAND SharedMemoryRegion)


###################################################################

# Target for building all tests
add_custom_target(tests
    DEPENDS SharedMemoryRegion)
