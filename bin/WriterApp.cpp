#include "WriterApp.hpp"

#include <atomic>
#include <csignal>
#include <functional>
#include <thread>

using namespace std::chrono_literals;

std::atomic_bool g_Running{true};
void stop(int) { g_Running.store(false, std::memory_order_release); }

int main(int argc, char *argv[]) {
    std::signal(SIGINT, stop);

    WriterApp app(argc, argv);
    std::thread thread(&WriterApp::Run, std::ref(app));

    while (g_Running.load(std::memory_order_acquire) && app.Running()) {
        std::this_thread::sleep_for(100ms);
    }

    app.Stop();
    thread.join();
}
