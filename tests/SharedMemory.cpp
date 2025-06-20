#include "SharedMemory.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstring>
#include <stdexcept>

#include "Utils.hpp"

const char *g_ValidName = "/testing";
const size_t g_Size = 1024 * 1024;  // 1 MiB

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

    // Create a second instance
    EXPECT_THROW(SharedMemory(g_ValidName, g_Size + 1), std::runtime_error);
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
