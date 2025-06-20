find_package(GTest REQUIRED)
enable_testing()

# Testing library
add_library(testing SHARED EXCLUDE_FROM_ALL Utils.cpp)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_libraries(asan circularbuffer testing GTest::gtest GTest::gtest_main)
add_compile_options(-fsanitize=address)

############################## UNIT TESTS #########################
# SemaphoreLock
add_executable(SemaphoreLockTests EXCLUDE_FROM_ALL SemaphoreLock.cpp)
add_test(NAME SemaphoreLockTests COMMAND SemaphoreLockTests)

# SharedMemory
add_executable(SharedMemoryTests EXCLUDE_FROM_ALL SharedMemory.cpp)
add_test(NAME SharedMemoryTests COMMAND SharedMemoryTests)

# Writer
add_executable(WriterTests EXCLUDE_FROM_ALL Writer.cpp)
add_test(NAME WriterTests COMMAND WriterTests)

# Reader
add_executable(ReaderTests EXCLUDE_FROM_ALL Reader.cpp)
add_test(NAME ReaderTests COMMAND ReaderTests)
###################################################################

# Target for building all unit tests
add_custom_target(UnitTests
    DEPENDS
        SemaphoreLockTests
        SharedMemoryTests
        WriterTests
        ReaderTests
)
