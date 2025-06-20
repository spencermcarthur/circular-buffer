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
    static constexpr double FAST_ARRIVAL_TIME_MILLIS = 25;
    static constexpr double NORMAL_ARRIVAL_TIME_MILLIS = 100;
    static constexpr double MEAN_MSG_SIZE = 2000;
    static constexpr double STDDEV_MSG_SIZE = 500;

public:
    explicit WriterApp(int argc, char* argv[], bool fast = false)
        : m_Writer(LoadSpec(argv[0])),
          m_IsFast((fast || (argc == 2 && std::strcmp(argv[1], "fast") == 0) ||
                    (argc == 3 && (std::strcmp(argv[1], "fast") == 0 ||
                                   std::strcmp(argv[2], "fast") == 0)))),
          m_WaitTimeGen(m_IsFast ? FAST_ARRIVAL_TIME_MILLIS
                                 : NORMAL_ARRIVAL_TIME_MILLIS) {}

    EXPLICIT_DELETE_CONSTRUCTORS(WriterApp);

    void Run() {
        m_Running = true;
        while (m_Running) {
            RandomMessage msg(m_MsgSizeGen());
            m_Writer.Write(msg.buf());
            std::this_thread::sleep_for(m_WaitTimeGen());
        }
    }

    [[nodiscard]] bool Running() const { return m_Running; }
    void Stop() { m_Running = false; }

private:
    CircularBuffer::Writer m_Writer;
    std::atomic_bool m_Running{false};
    const bool m_IsFast;

    MessageSizeGenerator m_MsgSizeGen{MEAN_MSG_SIZE, STDDEV_MSG_SIZE};
    WaitTimeGenerator m_WaitTimeGen;
};
