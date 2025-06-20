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
using CircularBuffer::SeqNumT;

TEST_F(Writer, Constructor) {
    // Construct successfully
    EXPECT_NO_THROW(CB::Writer writer{spec});
}

TEST_F(Writer, ConstructorFailIfMultipleWriters) {
    CB::Writer writer(spec);

    // Can't have multiple writers
    EXPECT_THROW(CB::Writer{spec}, std::logic_error);
}

TEST_F(Writer, WriteSingleMessage) {
    CB::Writer writer(spec);

    // Create write buffer
    const size_t msgSize = 128;
    BufferT writeBuffer = MakeBuffer(msgSize, '\1');

    // Record the initial write iterator position and expected position after
    // write
    const int bytesPerWrite = HEADER_SIZE + msgSize;
    const IndexT posBefore = state->writeIdx;
    const IndexT expectedPosAfter = (posBefore + bytesPerWrite) % bufferSize;

    // Perform write
    bool writeRes;
    EXPECT_NO_THROW(writeRes = writer.Write(writeBuffer));
    EXPECT_TRUE(writeRes);

    // Check index is where we expected
    EXPECT_EQ(state->readIdx, expectedPosAfter);
    EXPECT_EQ(state->writeIdx, expectedPosAfter);
    EXPECT_EQ(state->seqNum, bytesPerWrite);

    delete[] writeBuffer.data();
}

TEST_F(Writer, WrapAround) {
    CB::Writer writer(spec);

    // Create write buffer
    const size_t msgSize = MAX_MESSAGE_SIZE;
    BufferT writeBuffer = MakeBuffer(msgSize, '\1');

    // Compute expected position after wraparound
    const int bytesPerWrite = HEADER_SIZE + msgSize;
    const int writesToWrap = spec.bufferCapacity / bytesPerWrite + 1;
    const SeqNumT expectedSeqNumAfterWrap = bytesPerWrite * writesToWrap;
    const IndexT expectedPosAfterWrap = expectedSeqNumAfterWrap % bufferSize;

    // Perform writes to force wraparound
    bool writeRes;
    for (int i = 0; i < writesToWrap; i++) {
        EXPECT_NO_THROW(writeRes = writer.Write(writeBuffer));
        EXPECT_TRUE(writeRes);
    }

    // Check state is what we expected
    EXPECT_EQ(state->seqNum, expectedSeqNumAfterWrap);
    EXPECT_EQ(state->writeIdx, expectedPosAfterWrap);
    EXPECT_EQ(state->readIdx, expectedPosAfterWrap);

    delete[] writeBuffer.data();
}

// TODO: add a test for case where remaining space to the end of the buffer is
// too small for the header

TEST_F(Writer, WriteFailIfMessageTooBig) {
    CB::Writer writer(spec);

    // Create write buffer
    BufferT buffer = MakeBuffer(MAX_MESSAGE_SIZE + 1, '\1');
    EXPECT_FALSE(writer.Write(buffer));

    delete[] buffer.data();
}
