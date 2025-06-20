#include "circularbuffer/SemaphoreLock.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstring>
#include <stdexcept>

const char* g_ValidName = "/testing";

TEST(SemaphoreLock, Constructor) {
    // Construct semaphore lock on /testing
    EXPECT_NO_THROW(SemaphoreLock{g_ValidName});
}

TEST(SemaphoreLock, ConstructorFailIfNameBlank) {
    // Invalid name - 0 length
    EXPECT_THROW(SemaphoreLock{""}, std::length_error);
}

TEST(SemaphoreLock, ConstructorFailIfNameTooLong) {
    // Invalid name - too long
    char nameTooLong[SemaphoreLock::MAX_SEMAPHORE_NAME_LEN + 2]{};
    std::memset(nameTooLong, 'a', SemaphoreLock::MAX_SEMAPHORE_NAME_LEN + 1);
    EXPECT_THROW(SemaphoreLock{nameTooLong}, std::length_error);
}

TEST(SemaphoreLock, AcquireRelease) {
    // Construct 2 semaphore locks on /testing
    SemaphoreLock lock1(g_ValidName);
    SemaphoreLock lock2(g_ValidName);

    // sl1 locks, sl2 can't
    EXPECT_TRUE(lock1.Acquire());
    EXPECT_FALSE(lock2.Acquire());

    // sl1 unlocks, 2 can lock and unlock
    EXPECT_TRUE(lock1.Release());
    EXPECT_TRUE(lock2.Acquire());
    EXPECT_TRUE(lock2.Release());
}

TEST(SemaphoreLock, DestructorRelease) {
    // Construct and test
    SemaphoreLock lock1(g_ValidName);
    EXPECT_TRUE(lock1.Acquire());
    EXPECT_TRUE(lock1.Release());

    {
        // Construct another lock and acquire before destroying. Expected when
        // locked is to unlock on destruction.
        SemaphoreLock lock2(g_ValidName);
        EXPECT_TRUE(lock2.Acquire());
        EXPECT_FALSE(lock1.Acquire());
    }

    // First lock should be able to acquire
    EXPECT_TRUE(lock1.Acquire());
    EXPECT_TRUE(lock1.Release());
}
