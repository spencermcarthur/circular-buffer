#pragma once

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>
#include <thread>

#include "Common.hpp"
#include "Macros.hpp"
#include "circularbuffer/Aliases.hpp"
#include "circularbuffer/Reader.hpp"

class ReaderApp {
    static constexpr std::chrono::microseconds NORMAL_READER_DELAY{10};
    // 500 ms - very slow
    static constexpr std::chrono::microseconds SLOW_READER_DELAY{500'000};

public:
    explicit ReaderApp(int argc, char* argv[], bool slow = false)
        : m_Reader(LoadSpec(argv[0])),
          m_IsSlow(slow || (argc == 2 && strcmp(argv[1], "slow") == 0) ||
                   ((argc == 3 && (strcmp(argv[1], "slow") == 0 ||
                                   strcmp(argv[2], "slow") == 0)))),
          m_ProcessingDelayMics(m_IsSlow ? SLOW_READER_DELAY
                                         : NORMAL_READER_DELAY) {}

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
        std::this_thread::sleep_for(m_ProcessingDelayMics);
    }

    CircularBuffer::Reader m_Reader;
    std::atomic_bool m_Running{false};

    static constexpr size_t BUFSZ = CircularBuffer::MAX_MESSAGE_SIZE;
    CircularBuffer::DataT m_BufferData[BUFSZ]{};

    const bool m_IsSlow;
    const std::chrono::microseconds m_ProcessingDelayMics;
};
