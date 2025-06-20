#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <format>
#include <stdexcept>
#include <thread>

#include "Defines.hpp"
#include "Demo.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Reader.hpp"

class ReaderDemo {
    static constexpr uint64_t SLOW_READER_DELAY_MILLIS = 500;

public:
    explicit ReaderDemo(int argc, char* argv[])
        : m_Reader(LoadSpec(argc, argv)),
          m_BufferData(new CircularBuffer::DataT[m_BufferSize]{}),
          m_Slow((argc == 2 && strcmp(argv[1], "slow") == 0) ||
                 ((argc == 3 && strcmp(argv[2], "slow") == 0))) {}
    ~ReaderDemo() { delete[] m_BufferData; }

    EXPLICIT_DELETE_CONSTRUCTORS(ReaderDemo);

    void Run() {
        std::signal(SIGINT, ReaderDemo::Stop);
        sm_Running.store(true, std::memory_order_release);

        CircularBuffer::BufferT readBuffer(m_BufferData, m_BufferSize);
        while (sm_Running.load(std::memory_order_acquire)) {
            const int bytesRead = m_Reader.Read(readBuffer);

            if (bytesRead == 0) {
                continue;
            }

            if (bytesRead < 0) {
                if (bytesRead == -1) {
                    throw std::runtime_error(std::format("Buffer too small"));
                }
                if (bytesRead == INT_MIN) {
                    throw std::runtime_error(std::format("Overwritten"));
                }
            }

            SimulateProcessing(readBuffer);
        }
    }

private:
    static void Stop(int) {
        sm_Running.store(false, std::memory_order_release);
    }

    void SimulateProcessing(CircularBuffer::BufferT) const {
        if (m_Slow) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(SLOW_READER_DELAY_MILLIS));
        }
    }

    CircularBuffer::Reader m_Reader;
    inline static std::atomic_bool sm_Running = false;
    CircularBuffer::DataT* m_BufferData;
    static constexpr size_t m_BufferSize = CircularBuffer::MAX_MESSAGE_SIZE;
    const bool m_Slow;
};
