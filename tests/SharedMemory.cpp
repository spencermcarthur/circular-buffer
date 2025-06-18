#include "SharedMemory.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstring>
#include <stdexcept>

#include "Utils.hpp"

const char *g_ValidName = "/testing";
const size_t g_Size = 1024 * 1024;  // 1 MiB

TEST(SharedMemory, Construct) {
    // Unlink shared memory if it already exists
    if (CheckSharedMemExists(g_ValidName)) {
        UnlinkSharedMem(g_ValidName);
    }

    // Make sure it doesn't exist
    ASSERT_FALSE(CheckSharedMemExists(g_ValidName));

    // Create shared memory and verify that everything was initialized as
    // expected
    {
        SharedMemory shmem(g_ValidName, g_Size);

        EXPECT_STREQ(g_ValidName, shmem.Name().c_str());
        EXPECT_EQ(g_Size, shmem.Size());
        EXPECT_EQ(shmem.ReferenceCount(), 1);

        // Check that shared memory was created
        EXPECT_TRUE(CheckSharedMemExists(g_ValidName));
    }

    // Check that shared memory was unlinked again when shmem was destroyed
    EXPECT_FALSE(CheckSharedMemExists(g_ValidName));
}

TEST(SharedMemory, ConstructFailCases) {
    // Invalid name - too short
    EXPECT_THROW(SharedMemory("", 0), std::length_error);

    // Invalid name - too long
    char nameTooLong[SharedMemory::MAX_NAME_LEN + 2];
    std::memset(nameTooLong, 'a', SharedMemory::MAX_NAME_LEN + 1);
    EXPECT_THROW(SharedMemory(nameTooLong, 0), std::length_error);

    // Invalid size - 0
    EXPECT_THROW(SharedMemory(g_ValidName, 0), std::domain_error);

    // Invalid size - too large
    EXPECT_THROW(
        SharedMemory(g_ValidName, SharedMemory::MAX_SHARED_MEM_SIZE_BYTES + 1),
        std::domain_error);
}

TEST(SharedMemory, ConstructMultiple) {
    // Unlink shared memory if it already exists
    if (CheckSharedMemExists(g_ValidName)) {
        UnlinkSharedMem(g_ValidName);
    }

    // Make sure it doesn't exist
    ASSERT_FALSE(CheckSharedMemExists(g_ValidName));

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

    // Check that shared memory was unlinked again when shmem was destroyed
    EXPECT_FALSE(CheckSharedMemExists(g_ValidName));
}
