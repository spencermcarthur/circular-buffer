#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <thread>

#include "Common.hpp"
#include "Macros.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Reader.hpp"

class ReaderApp {
    static constexpr uint64_t SLOW_READER_DELAY_MILLIS = 500;

public:
    explicit ReaderApp(int argc, char* argv[], bool slow = false)
        : m_Reader(LoadSpec(argc, argv)),
          m_Slow(slow || (argc == 2 && strcmp(argv[1], "slow") == 0) ||
                 ((argc == 3 && strcmp(argv[2], "slow") == 0))) {}

    EXPLICIT_DELETE_CONSTRUCTORS(ReaderApp);

    void Run() {
        CircularBuffer::BufferT readBuffer(m_BufferData, BUFSZ);
        m_Running = true;

        while (m_Running) {
            const int bytesRead = m_Reader.Read(readBuffer);

            // Nothing to read
            if (bytesRead == 0) {
                continue;
            }

            // Error
            if (bytesRead < 0) {
                break;
            }

            // "Process" message
            SimulateProcessing(readBuffer);
        }

        m_Running = false;
    }

    [[nodiscard]] bool Running() const { return m_Running; }
    void Stop() { m_Running = false; }

private:
    void SimulateProcessing(CircularBuffer::BufferT) const {
        if (m_Slow) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(SLOW_READER_DELAY_MILLIS));
        }
    }

    CircularBuffer::Reader m_Reader;
    std::atomic_bool m_Running{false};
    static constexpr size_t BUFSZ = CircularBuffer::MAX_MESSAGE_SIZE;
    CircularBuffer::DataT m_BufferData[BUFSZ]{};
    const bool m_Slow;
};
