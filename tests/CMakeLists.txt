enable_testing()

find_package(GTest REQUIRED)

# Testing library
add_library(Testing SHARED EXCLUDE_FROM_ALL
    Utils.cpp)

link_libraries(CircularBuffer Testing GTest::gtest GTest::gtest_main)

############################## TESTS ##############################
# SemLock
add_executable(SemLock EXCLUDE_FROM_ALL SemLock.cpp)
add_test(NAME SemLock COMMAND SemLock)

# SharedMemory
add_executable(SharedMemory EXCLUDE_FROM_ALL SharedMemory.cpp)
add_test(NAME SharedMemory COMMAND SharedMemory)
###################################################################

# Target for building all tests
add_custom_target(tests
    DEPENDS
        SharedMemory
        SemLock
)
