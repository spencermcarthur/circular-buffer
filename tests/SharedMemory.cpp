#include "SharedMemory.hpp"
#include "Utils.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

const char *g_Name = "/testing";
const size_t g_Size = 1024 * 1024; // 1 MiB

TEST(SharedMemory, Construct) {
  // Unlink shared memory if it already exists
  if (CheckSharedMemExists(g_Name))
    UnlinkSharedMem(g_Name);

  // Make sure it doesn't exist
  ASSERT_FALSE(CheckSharedMemExists(g_Name));

  // Create shared memory and verify that everything was initialized as expected
  {
    SharedMemory shmem(g_Name, g_Size);

    EXPECT_STREQ(g_Name, shmem.Name().c_str());
    EXPECT_EQ(g_Size, shmem.Size());
    EXPECT_EQ(shmem.ReferenceCount(), 1);

    // Check that shared memory was created
    EXPECT_TRUE(CheckSharedMemExists(g_Name));
  }

  // Check that shared memory was unlinked again when shmem was destroyed
  EXPECT_FALSE(CheckSharedMemExists(g_Name));
}

TEST(SharedMemory, ConstructFailCases) {
  // Name length
  EXPECT_THROW(SharedMemory("", 0), std::length_error);
  // Requested size
  EXPECT_THROW(SharedMemory(g_Name, 0), std::domain_error);
}

TEST(SharedMemory, ConstructMultiple) {
  // Unlink shared memory if it already exists
  if (CheckSharedMemExists(g_Name))
    UnlinkSharedMem(g_Name);

  // Make sure it doesn't exist
  ASSERT_FALSE(CheckSharedMemExists(g_Name));

  // Construct multiple shared memory regions and verify that reference counter
  // is increasing and decreasing as expected
  {
    SharedMemory shmem1(g_Name, g_Size);
    EXPECT_EQ(shmem1.ReferenceCount(), 1);

    {
      SharedMemory shmem2(g_Name, g_Size);
      EXPECT_EQ(shmem1.ReferenceCount(), 2);

      {
        SharedMemory shmem3(g_Name, g_Size);

        EXPECT_EQ(shmem1.ReferenceCount(), 3);
      }

      EXPECT_EQ(shmem1.ReferenceCount(), 2);
    }

    EXPECT_EQ(shmem1.ReferenceCount(), 1);
  }

  // Check that shared memory was unlinked again when shmem was destroyed
  EXPECT_FALSE(CheckSharedMemExists(g_Name));
}
