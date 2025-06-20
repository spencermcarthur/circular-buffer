#include "circularbuffer/Writer.hpp"

#include <benchmark/benchmark.h>

#include <cstring>

#include "SharedMemory.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Spec.hpp"
#include "spdlog/common.h"
#include "spdlog/spdlog.h"

using namespace CircularBuffer;

void BM_Write(benchmark::State& state) {
    // Disable logging
    spdlog::set_level(spdlog::level::off);

    // Set up write buffer
    const size_t msgSize = state.range(0);
    DataT* msgData = new DataT[msgSize]{};
    std::memset(msgData, '\1', msgSize);
    BufferT writeBuffer(msgData, msgSize);

    // Set up writer
    const size_t bufferSize = state.range(1);
    Spec spec{"/bench-index", "/bench-data", bufferSize};
    Writer writer(spec);

    // Benchmark
    for (auto _ : state) {
        writer.Write(writeBuffer);
    }
    state.SetItemsProcessed(state.iterations());
    state.SetBytesProcessed(state.iterations() * writeBuffer.size_bytes());

    // Free buffer data
    delete[] msgData;
}

BENCHMARK(BM_Write)->Ranges({
    {1, MAX_MESSAGE_SIZE},  // Message size range
    {HEADER_SIZE + MAX_MESSAGE_SIZE,
     SharedMemory::MAX_SIZE_BYTES},  // Buffer size range
});

BENCHMARK_MAIN();
