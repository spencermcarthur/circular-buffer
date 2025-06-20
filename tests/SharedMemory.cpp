#include "SharedMemory.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstddef>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string_view>

#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"

const char *g_ValidName = "/testing";
const size_t g_Size = 1024 * 1024;  // 1 MiB

using CircularBuffer::DataT;

TEST(SharedMemory, ConstructorNewMemory) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Create shared memory and verify that everything was initialized as
    // expected
    {
        SharedMemory shmem(g_ValidName, g_Size);

        EXPECT_STREQ(g_ValidName, shmem.Name().c_str());
        EXPECT_EQ(g_Size, shmem.Size());
        EXPECT_EQ(shmem.ReferenceCount(), 1);

        // Check that shared memory was created
        EXPECT_TRUE(SharedMemExists(g_ValidName));
    }
}

TEST(SharedMemory, ConstructorExistingMemory) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Create an instance of SharedMemory
    SharedMemory shmem1(g_ValidName, g_Size);

    // Check that shared memory was created
    EXPECT_TRUE(SharedMemExists(g_ValidName));

    // Make sure we are the only instance
    EXPECT_EQ(shmem1.ReferenceCount(), 1);

    {
        // Create a second instance
        SharedMemory shmem2(g_ValidName, g_Size);

        // Verify name/size
        EXPECT_STREQ(g_ValidName, shmem2.Name().c_str());
        EXPECT_EQ(g_Size, shmem2.Size());

        // Make sure ref count increased
        EXPECT_EQ(shmem2.ReferenceCount(), 2);
    }

    // Make sure we didn't free the memory
    EXPECT_TRUE(SharedMemExists(g_ValidName));

    // Make sure ref count decreased
    EXPECT_EQ(shmem1.ReferenceCount(), 1);
}

TEST(SharedMemory, DestructorFreeMemoryOnRefCountZero) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    {
        // Create shared memory
        SharedMemory shmem(g_ValidName, g_Size);

        // Check that shared memory was created
        EXPECT_TRUE(SharedMemExists(g_ValidName));

        // Verify that we have the only handle
        EXPECT_EQ(shmem.ReferenceCount(), 1);
    }

    // Check that shared memory was unlinked again when shmem was destroyed
    EXPECT_FALSE(SharedMemExists(g_ValidName));
}

TEST(SharedMemory, ConstructorFailInvalidSizeRequested) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Size 0
    EXPECT_THROW(SharedMemory(g_ValidName, 0), std::domain_error);

    // Size too big
    EXPECT_THROW(SharedMemory(g_ValidName, SharedMemory::MAX_SIZE_BYTES + 1),
                 std::domain_error);

    // Check that shared memory was not created
    EXPECT_FALSE(SharedMemExists(g_ValidName));
}

TEST(SharedMemory, ConstructorFailExistingMemoryWrongSize) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Create an instance of SharedMemory
    SharedMemory shmem1(g_ValidName, g_Size);

    // Check that shared memory was created
    EXPECT_TRUE(SharedMemExists(g_ValidName));

    // Make sure we are the only instance
    EXPECT_EQ(shmem1.ReferenceCount(), 1);

    // Try to create a second instance with same name, different size
    EXPECT_THROW(SharedMemory(g_ValidName, g_Size + 1), std::runtime_error);
}

TEST(SharedMemory, ConstructorFailInvalidName) {
    // Make sure shared memory doesn't exist
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Try to create with blank name
    EXPECT_THROW(SharedMemory("", g_Size), std::length_error);

    // Try to create with name too long
    char name[SharedMemory::MAX_NAME_LEN + 2]{};
    std::memset(name, 'a', SharedMemory::MAX_NAME_LEN + 1);
    EXPECT_THROW(SharedMemory(name, g_Size), std::length_error);
}

TEST(SharedMemory, ConstructMultiple) {
    // Unlink shared memory if it already exists
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }

    // Make sure it doesn't exist
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Construct multiple shared memory regions and verify that reference
    // counter is increasing and decreasing as expected
    {
        SharedMemory shmem1(g_ValidName, g_Size);
        EXPECT_EQ(shmem1.ReferenceCount(), 1);

        {
            SharedMemory shmem2(g_ValidName, g_Size);
            EXPECT_EQ(shmem1.ReferenceCount(), 2);

            {
                SharedMemory shmem3(g_ValidName, g_Size);
                EXPECT_EQ(shmem1.ReferenceCount(), 3);
            }

            EXPECT_EQ(shmem1.ReferenceCount(), 2);
        }

        EXPECT_EQ(shmem1.ReferenceCount(), 1);
    }

    // Check that shared memory was unlinked again when shmem1 was destroyed
    EXPECT_FALSE(SharedMemExists(g_ValidName));
}

TEST(SharedMemory, ConstructMultipleMemoryActuallyShared) {
    // Unlink shared memory if it already exists
    if (SharedMemExists(g_ValidName)) {
        FreeSharedMem(g_ValidName);
    }

    // Make sure it doesn't exist
    ASSERT_FALSE(SharedMemExists(g_ValidName));

    // Construct multiple shared memory regions and verify that the memory is
    // changed when written
    SharedMemory shmem1(g_ValidName, g_Size);
    EXPECT_EQ(shmem1.ReferenceCount(), 1);

    // Get a span
    std::span<DataT> span1 = shmem1.AsSpan<DataT>();

    // Verify memory initialized correctly
    EXPECT_EQ(span1.size(), g_Size);
    for (DataT byte : span1) {
        EXPECT_EQ(byte, std::byte('\0'));
    }

    // Set the memory to 'a' and verify
    std::memset(span1.data(), 'a', span1.size());
    for (DataT byte : span1) {
        EXPECT_EQ(byte, std::byte('a'));
    }

    {
        // Create new shared memory on the same data
        SharedMemory shmem2(g_ValidName, g_Size);
        EXPECT_EQ(shmem1.ReferenceCount(), 2);
        EXPECT_EQ(shmem2.ReferenceCount(), 2);

        // Get a another span
        std::span<DataT> span2 = shmem1.AsSpan<DataT>();
        EXPECT_EQ(span1.size(), span2.size());

        // Verify the data is all 'a'
        for (DataT byte : span2) {
            EXPECT_EQ(byte, std::byte('a'));
        }

        // Set the data to 'b'
        std::memset(span2.data(), 'b', span2.size());

        // Verify
        for (DataT byte : span2) {
            EXPECT_EQ(byte, std::byte('b'));
        }
    }

    // shmem2 goes out of scope
    EXPECT_EQ(shmem1.ReferenceCount(), 1);

    // Verify span1 was updated to 'b'
    for (DataT byte : span1) {
        EXPECT_EQ(byte, std::byte('b'));
    }
}
