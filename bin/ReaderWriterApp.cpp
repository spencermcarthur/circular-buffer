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
    std::signal(SIGINT, Stop);

    ReaderApp reader(argc, argv);
    WriterApp writer(argc, argv);

    std::thread readerThread(&ReaderApp::Run, std::ref(reader));
    std::thread writerThread(&WriterApp::Run, std::ref(writer));

    // Wait for reader to start
    while (!reader.Running()) {
        std::this_thread::sleep_for(1s);
    }

    // Keep running until signaled or reader errors
    while (g_Running && reader.Running()) {
        std::this_thread::sleep_for(100ms);
    }

    writer.Stop();
    reader.Stop();

    writerThread.join();
    readerThread.join();
}
