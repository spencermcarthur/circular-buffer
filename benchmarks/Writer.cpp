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

    for (auto iter : state) {
        writer.Write(writeBuffer);
        benchmark::ClobberMemory();
    }

    delete[] msgData;

    state.SetBytesProcessed(state.iterations() * msgSize);
}

BENCHMARK(BM_Write)->Ranges({
    {1, MAX_MESSAGE_SIZE},  // Message size
    {Writer::MIN_BUFFER_SIZE_BYTES,
     SharedMemory::MAX_SIZE_BYTES},  // Buffer size
});

BENCHMARK_MAIN();
