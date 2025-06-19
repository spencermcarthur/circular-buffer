#pragma once

#include <gtest/gtest.h>

#include <cstring>

#include "SharedMemory.hpp"
#include "circularbuffer/Spec.hpp"
#include "circularbuffer/State.hpp"
#include "circularbuffer/Writer.hpp"

namespace CB = CircularBuffer;

class Reader : public ::testing::Test {
protected:
    static constexpr size_t bufferSize = 1024 * 1024;

    void SetUp() override {
        spec = CB::Spec{"/testing-index", "/testing-data", bufferSize};
        writer = new CB::Writer(spec);
        m_StateShMem =
            new SharedMemory(spec.indexSharedMemoryName, sizeof(CB::State));
        state = m_StateShMem->AsStruct<CB::State>();
    }

    void TearDown() override {
        state = nullptr;
        delete m_StateShMem;
        delete writer;
    }

    CB::Spec spec;
    CB::Writer* writer{nullptr};
    CB::State* state{nullptr};

private:
    SharedMemory* m_StateShMem{nullptr};
};
