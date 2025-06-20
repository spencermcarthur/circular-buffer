#pragma once

#include <atomic>
#include <csignal>
#include <thread>

#include "Macros.hpp"
#include "Demo.hpp"
#include "Generators.hpp"
#include "circularbuffer/Writer.hpp"

class WriterApp {
    static constexpr double FAST_ARRIVAL_RATE = 50;
    static constexpr double NORMAL_ARRIVAL_RATE = 200;
    static constexpr double SLOW_ARRIVAL_RATE = 500;
    static constexpr double MEAN_MSG_SIZE = 300;

public:
    explicit WriterApp(int argc, char* argv[])
        : m_Writer(LoadSpec(argc, argv)) {}

    EXPLICIT_DELETE_CONSTRUCTORS(WriterApp);

    void Run() {
        std::signal(SIGINT, WriterApp::Stop);
        sm_Running.store(true, std::memory_order_release);

        while (sm_Running.load(std::memory_order_acquire)) {
            RandomMessage msg(m_MsgSizeGen());
            m_Writer.Write(msg.buf());
            std::this_thread::sleep_for(m_WaitTimeGen());
        }
    }

private:
    static void Stop(int) {
        sm_Running.store(false, std::memory_order_release);
    }

    CircularBuffer::Writer m_Writer;
    WaitTimeGenerator m_WaitTimeGen{NORMAL_ARRIVAL_RATE};
    MessageSizeGenerator m_MsgSizeGen{MEAN_MSG_SIZE};
    inline static std::atomic_bool sm_Running = false;
};
