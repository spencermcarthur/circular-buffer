#pragma once

#include <gtest/gtest.h>

#include <cstring>

#include "SharedMemory.hpp"
#include "circularbuffer/Spec.hpp"
#include "circularbuffer/State.hpp"

class Writer : public testing::Test {
protected:
    static constexpr size_t bufferSize = 1024 * 1024;

    void SetUp() override {
        spec =
            CircularBuffer::Spec{"/testing-index", "/testing-data", bufferSize};
        m_StateShMem = new SharedMemory(spec.indexSharedMemoryName,
                                        sizeof(CircularBuffer::State));
        state = m_StateShMem->AsStruct<CircularBuffer::State>();
    }

    void TearDown() override {
        state = nullptr;
        delete m_StateShMem;
    }

    CircularBuffer::Spec spec;
    CircularBuffer::State* state{nullptr};

private:
    SharedMemory* m_StateShMem{nullptr};
};
