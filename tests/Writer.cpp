#include "circularbuffer/Writer.hpp"

#include <gtest/gtest.h>

#include <cstring>
#include <stdexcept>

#include "Utils.hpp"
#include "Writer.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Spec.hpp"
#include "circularbuffer/State.hpp"

namespace CB = CircularBuffer;
using CB::BufferT;
using CB::DataT;
using CB::HEADER_SIZE;
using CB::IndexT;
using CB::MAX_MESSAGE_SIZE;

TEST_F(Writer, Constructor) {
    // Construct successfully
    EXPECT_NO_THROW(CB::Writer writer{spec});
}

TEST_F(Writer, ConstructorFailIfMultipleWriters) {
    CB::Writer writer(spec);

    // Can't have multiple writers
    EXPECT_THROW(CB::Writer{spec}, std::logic_error);
}

TEST_F(Writer, WriteSome) {
    CB::Writer writer(spec);

    // Record the initial write iterator position
    const IndexT posBefore = state->writeIdx;

    // Create write buffer
    const size_t msgSize = 128;
    BufferT buffer = MakeBuffer(msgSize, '\1');

    const int bytesPerWrite = HEADER_SIZE + msgSize;

    // Perform write
    bool writeRes;
    EXPECT_NO_THROW(writeRes = writer.Write(buffer));
    EXPECT_TRUE(writeRes);

    // Check index is where we expected
    const IndexT posAfter = state->writeIdx;
    const IndexT expectedPosAfter = posBefore + bytesPerWrite;

    EXPECT_EQ(posAfter, expectedPosAfter);

    delete[] buffer.data();
}

TEST_F(Writer, Wraparound) {
    CB::Writer writer(spec);

    // Create write buffer
    const size_t msgSize = MAX_MESSAGE_SIZE;
    BufferT buffer = MakeBuffer(msgSize, '\1');
    const int totalBytesPerWrite = HEADER_SIZE + msgSize;

    // Number of writes we need to perform to force a wraparound
    const int numWritesToForceWraparound =
        spec.bufferCapacity / (totalBytesPerWrite) + 1;
    const IndexT expectedPosAfter = totalBytesPerWrite;

    // Perform writes to force wraparound
    bool writeRes;
    for (int i = 0; i < numWritesToForceWraparound; i++) {
        EXPECT_NO_THROW(writeRes = writer.Write(buffer));
        EXPECT_TRUE(writeRes);
    }

    // Check index is where we expected
    EXPECT_EQ(state->writeIdx, expectedPosAfter);
    EXPECT_EQ(state->readIdx, expectedPosAfter);

    delete[] buffer.data();
}

TEST_F(Writer, WriteFailIfMessageTooBig) {
    CB::Writer writer(spec);

    // Create write buffer
    BufferT buffer = MakeBuffer(MAX_MESSAGE_SIZE + 1, '\1');
    EXPECT_FALSE(writer.Write(buffer));

    delete[] buffer.data();
}
