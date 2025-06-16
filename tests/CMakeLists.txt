find_package(GTest REQUIRED)

# Unit tests for Message
add_executable(MessageTests EXCLUDE_FROM_ALL Message.cpp)
target_link_libraries(MessageTests CircularBuffer GTest::GTest GTest::Main)

# Unit tests for CircularBuffer
add_executable(CircularBufferTests EXCLUDE_FROM_ALL CircularBuffer.cpp)
target_link_libraries(CircularBufferTests CircularBuffer GTest::GTest GTest::Main)

add_test(NAME TestMessage COMMAND MessageTests)
add_test(NAME TestCircularBuffer COMMAND CircularBufferTests)
