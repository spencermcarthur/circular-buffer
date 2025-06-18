enable_testing()

find_package(GTest REQUIRED)

# Testing library
add_library(Testing SHARED EXCLUDE_FROM_ALL
    Utils.cpp)

link_libraries(CircularBuffer Testing GTest::gtest GTest::gtest_main)

############################## TESTS ##############################
# SemaphoreLock
add_executable(SemaphoreLock EXCLUDE_FROM_ALL SemaphoreLock.cpp)
add_test(NAME SemaphoreLock COMMAND SemaphoreLock)

# SharedMemory
add_executable(SharedMemory EXCLUDE_FROM_ALL SharedMemory.cpp)
add_test(NAME SharedMemory COMMAND SharedMemory)

# Writer
add_executable(Writer EXCLUDE_FROM_ALL Writer.cpp)
add_test(NAME Writer COMMAND Writer)

# Reader
add_executable(Reader EXCLUDE_FROM_ALL Reader.cpp)
add_test(NAME Reader COMMAND Reader)
###################################################################

# Target for building all tests
add_custom_target(tests
    DEPENDS
        SemaphoreLock
        SharedMemory
        Writer
        Reader
)
