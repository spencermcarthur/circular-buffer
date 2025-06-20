#include "SemaphoreLock.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstring>
#include <stdexcept>

const char* g_ValidName = "/testing";

TEST(SemaphoreLock, Constructor) {
    // Construct semaphore lock on /testing
    EXPECT_NO_THROW(SemaphoreLock{g_ValidName});
}

TEST(SemaphoreLock, ConstructorFailIfNameTooShort) {
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
    SemaphoreLock sl1(g_ValidName);
    SemaphoreLock sl2(g_ValidName);

    // sl1 locks, sl2 can't
    EXPECT_TRUE(sl1.Acquire());
    EXPECT_FALSE(sl2.Acquire());

    // sl1 unlocks, 2 can lock and unlock
    EXPECT_TRUE(sl1.Release());
    EXPECT_TRUE(sl2.Acquire());
    EXPECT_TRUE(sl2.Release());
}

TEST(SemaphoreLock, DestructorRelease) {
    // Construct and test
    SemaphoreLock sl1(g_ValidName);
    EXPECT_TRUE(sl1.Acquire());
    EXPECT_TRUE(sl1.Release());

    {
        // Construct another lock and acquire before destroying. Expected when
        // locked is to unlock on destruction.
        SemaphoreLock sl2(g_ValidName);
        EXPECT_TRUE(sl2.Acquire());
    }

    // First lock should be able to acquire
    EXPECT_TRUE(sl1.Acquire());
    EXPECT_TRUE(sl1.Release());
}
