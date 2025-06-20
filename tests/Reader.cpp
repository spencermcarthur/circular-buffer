#include "circularbuffer/Reader.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <cstring>

#include "Reader.hpp"
#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"

namespace CB = CircularBuffer;
using CB::BufferT;
using CB::DataT;
using CB::HEADER_SIZE;
using CB::MAX_MESSAGE_SIZE;

TEST_F(Reader, Constructor) {
    // Construct successfully
    CB::Reader reader(spec);
}

TEST_F(Reader, ReadNothing) {
    constexpr int iter = 1000;

    CB::Reader reader(spec);

    BufferT buffer = MakeBuffer(MAX_MESSAGE_SIZE);
    for (int i = 0; i < iter; i++) {
        EXPECT_EQ(reader.Read(buffer), 0);
        EXPECT_EQ(state->readIdx, 0);
        EXPECT_EQ(state->writeIdx, 0);
    }

    delete[] buffer.data();
}

TEST_F(Reader, ReadSome) {
    CB::Reader reader(spec);

    // Make a buffer for r/w
    const int msgSize = 128;
    BufferT writeBuffer = MakeBuffer(msgSize, '\1');
    const int bytesToWrite = msgSize + HEADER_SIZE;

    // Check indices
    EXPECT_EQ(state->readIdx, 0);
    EXPECT_EQ(state->writeIdx, 0);

    // Write
    EXPECT_NO_THROW(writer->Write(writeBuffer));

    // Check indices moved after write
    EXPECT_EQ(state->readIdx, bytesToWrite);
    EXPECT_EQ(state->writeIdx, bytesToWrite);

    // Reset buffer
    BufferT readBuffer = MakeBuffer(MAX_MESSAGE_SIZE);

    // Read and verify
    EXPECT_EQ(reader.Read(readBuffer), msgSize);
    for (size_t i = 0; i < writeBuffer.size(); i++) {
        EXPECT_EQ(readBuffer[i], writeBuffer[i]);
    }

    delete[] readBuffer.data();
    delete[] writeBuffer.data();
}

TEST_F(Reader, Wraparoud) {
    CB::Reader reader(spec);

    // Set up buffers
    const int msgSize = MAX_MESSAGE_SIZE;
    BufferT writeBuffer = MakeBuffer(msgSize, '\1');
    BufferT readBuffer = MakeBuffer(msgSize, '\0');

    const int bytesPerWrite = HEADER_SIZE + msgSize;
    const int writesToWrap = bufferSize / bytesPerWrite + 1;
    const int expectedPosAfter = (writesToWrap * bytesPerWrite) % bufferSize;

    // Read and write
    for (int i = 0; i < writesToWrap - 1; i++) {
        writer->Write(writeBuffer);
        EXPECT_EQ(state->writeIdx, (i + 1) * bytesPerWrite);
        EXPECT_EQ(state->readIdx, (i + 1) * bytesPerWrite);

        EXPECT_EQ(reader.Read(readBuffer), msgSize);
    }

    // Wraparound write
    const int newValue = 'x';
    std::memset(writeBuffer.data(), newValue, writeBuffer.size());
    writer->Write(writeBuffer);
    EXPECT_EQ(state->writeIdx, expectedPosAfter);
    EXPECT_EQ(state->readIdx, expectedPosAfter);

    // Read and verify
    EXPECT_EQ(reader.Read(readBuffer), msgSize);
    for (DataT byte : readBuffer) {
        EXPECT_EQ(byte, static_cast<DataT>(newValue));
    }

    delete[] writeBuffer.data();
    delete[] readBuffer.data();
}

// TODO:

TEST_F(Reader, ReadFailBufferTooSmall) {
    CB::Reader reader(spec);

    // Set up buffers
    const int msgSize = 128;
    BufferT readBuffer = MakeBuffer(msgSize / 2);
    BufferT writeBuffer = MakeBuffer(msgSize, '\1');

    // Write
    writer->Write(writeBuffer);
    EXPECT_EQ(state->writeIdx, msgSize + HEADER_SIZE);
    EXPECT_EQ(state->readIdx, msgSize + HEADER_SIZE);

    // Expect to fail
    EXPECT_EQ(reader.Read(readBuffer), -1);

    delete[] readBuffer.data();
    delete[] writeBuffer.data();
}
