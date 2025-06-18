#include "circularbuffer/Writer.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstring>
#include <stdexcept>

#include "SharedMemory.hpp"
#include "Utils.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Indices.hpp"
#include "circularbuffer/Spec.hpp"

using CircularBuffer::BufferT;
using CircularBuffer::DataT;
using CircularBuffer::IndexT;
using CircularBuffer::Indices;
using CircularBuffer::MessageSizeT;
using CircularBuffer::Spec;
using CircularBuffer::Writer;

static const Spec g_Spec = {
    "/testing-index",
    "/testing-buffer",
    1024 * 1024,
};

class TestWriter : public Writer {};

TEST(Writer, Constructor) {
    // Release shared memory if they exist
    if (SharedMemExists(g_Spec.indexSharedMemoryName.c_str())) {
        FreeSharedMem(g_Spec.indexSharedMemoryName.c_str());
    }
    if (SharedMemExists(g_Spec.dataSharedMemoryName.c_str())) {
        FreeSharedMem(g_Spec.dataSharedMemoryName.c_str());
    }

    // Construct successfully
    EXPECT_NO_THROW(Writer writer(g_Spec));

    // Make sure shared memory was release
    EXPECT_FALSE(SharedMemExists(g_Spec.indexSharedMemoryName.c_str()));
    EXPECT_FALSE(SharedMemExists(g_Spec.dataSharedMemoryName.c_str()));
}

TEST(Writer, Write) {
    Writer writer(g_Spec);

    // Access buffer indices
    SharedMemory shmem(g_Spec.indexSharedMemoryName, sizeof(Indices));
    Indices* indices = shmem.AsStruct<Indices>();

    // Record the initial write iterator position
    const IndexT posBefore = indices->write.load(std::memory_order_acquire);

    // Create write buffer
    const size_t size = 128;
    DataT data[size];
    std::memset(data, 'b', size);
    BufferT buffer(data, size);

    // Perform write
    EXPECT_NO_THROW(writer.Write(buffer));

    const IndexT posAfter = indices->write.load(std::memory_order_acquire);
    const IndexT expectedPosAfter = posBefore + size + sizeof(MessageSizeT);

    EXPECT_EQ(posAfter, expectedPosAfter);
}

TEST(Writer, FailIfBufferTooSmall) {
    Spec spec = g_Spec;

    // Smallest buffer allowed
    spec.bufferCapacity = Writer::MIN_BUFFER_SIZE;
    EXPECT_NO_THROW(Writer{spec});

    // Buffer too small
    spec.bufferCapacity = Writer::MIN_BUFFER_SIZE - 1;
    EXPECT_THROW(Writer{spec}, std::domain_error);
}

TEST(Writer, FailIfMultipleWriters) {
    Writer writer(g_Spec);

    // Can't have multiple writers
    EXPECT_THROW(Writer{g_Spec}, std::logic_error);
}
