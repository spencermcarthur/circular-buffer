#include "SemLock.hpp"

#include <gtest/gtest.h>
#include <linux/limits.h>

#include <cstring>
#include <stdexcept>

const char* g_ValidName = "/testing";

TEST(SemLock, Construct) {
    // Construct semaphore lock on /testing
    EXPECT_NO_THROW(SemLock{g_ValidName});
}

TEST(SemLock, ConstructFailCases) {
    // Invalid name - 0 length
    EXPECT_THROW(SemLock{""}, std::length_error);

    // Invalid name - too long
    char nameTooLong[SemLock::MAX_SEM_NAME_LEN + 2]{};
    std::memset(nameTooLong, 'a', SemLock::MAX_SEM_NAME_LEN + 1);
    EXPECT_THROW(SemLock{nameTooLong}, std::length_error);
}

TEST(SemLock, AcquireRelease) {
    // Construct 2 semaphore locks on /testing
    SemLock sl1(g_ValidName);
    SemLock sl2(g_ValidName);

    // sl1 locks, sl2 can't
    EXPECT_TRUE(sl1.Acquire());
    EXPECT_FALSE(sl2.Acquire());

    // sl1 unlocks, 2 can lock and unlock
    EXPECT_TRUE(sl1.Release());
    EXPECT_TRUE(sl2.Acquire());
    EXPECT_TRUE(sl2.Release());
}
