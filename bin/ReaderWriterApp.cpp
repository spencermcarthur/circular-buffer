#include <atomic>
#include <csignal>
#include <functional>
#include <thread>

#include "ReaderApp.hpp"
#include "WriterApp.hpp"

using namespace std::chrono_literals;

using namespace CircularBuffer;

std::atomic_bool g_Running{true};
void Stop(int) { g_Running.store(false, std::memory_order_release); }

int main(int argc, char* argv[]) {
    ReaderApp reader(argc, argv, true);
    WriterApp writer(argc, argv, true);

    std::thread readerThread(&ReaderApp::Run, std::ref(reader));
    std::thread writerThread(&WriterApp::Run, std::ref(writer));

    std::signal(SIGINT, Stop);

    while (g_Running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(100ms);

        if (!(reader.Running() && writer.Running())) {
            break;
        }
    }

    writer.Stop();
    reader.Stop();

    writerThread.join();
    readerThread.join();
}
