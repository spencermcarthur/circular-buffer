#pragma once

#include <atomic>
#include <csignal>
#include <cstring>
#include <thread>

#include "Common.hpp"
#include "Generators.hpp"
#include "Macros.hpp"
#include "circularbuffer/Writer.hpp"

class WriterApp {
    static constexpr double FAST_ARRIVAL_RATE = 50;
    static constexpr double NORMAL_ARRIVAL_RATE = 100;
    static constexpr double MEAN_MSG_SIZE = 300;

public:
    explicit WriterApp(int argc, char* argv[], bool fast = false)
        : m_Writer(LoadSpec(argc, argv)),
          m_Fast((fast || (argc == 2 && std::strcmp(argv[1], "fast") == 0) ||
                  (argc == 3 && std::strcmp(argv[2], "fast") == 0))),
          m_WaitTimeGen(m_Fast ? FAST_ARRIVAL_RATE : NORMAL_ARRIVAL_RATE) {}

    EXPLICIT_DELETE_CONSTRUCTORS(WriterApp);

    void Run() {
        m_Running = true;

        while (m_Running.load(std::memory_order_acquire)) {
            RandomMessage msg(m_MsgSizeGen());
            m_Writer.Write(msg.buf());
            std::this_thread::sleep_for(m_WaitTimeGen());
        }
    }

    [[nodiscard]] bool Running() const {
        return m_Running.load(std::memory_order_acquire);
    }
    void Stop() { m_Running.store(false, std::memory_order_release); }

private:
    CircularBuffer::Writer m_Writer;
    MessageSizeGenerator m_MsgSizeGen{MEAN_MSG_SIZE};
    std::atomic_bool m_Running{false};
    const bool m_Fast;
    WaitTimeGenerator m_WaitTimeGen;
};
