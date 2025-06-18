#include "SemLock.hpp"

#include <gtest/gtest.h>

const char *g_name = "/testing";

TEST(SemLock, Construct) {
  // Construct semaphore lock on /testing
  EXPECT_NO_THROW(SemLock{g_name});
}

TEST(SemLock, ConstructFailCases) {
  // Invalid name
  EXPECT_ANY_THROW(SemLock{""});
}

TEST(SemLock, AcquireRelease) {
  // Construct 2 semaphore locks on /testing
  SemLock sl1(g_name);
  SemLock sl2(g_name);

  // sl1 locks, sl2 can't
  EXPECT_TRUE(sl1.Acquire());
  EXPECT_FALSE(sl2.Acquire());

  // sl1 unlocks, 2 can lock and unlock
  EXPECT_TRUE(sl1.Release());
  EXPECT_TRUE(sl2.Acquire());
  EXPECT_TRUE(sl2.Release());
}
