#include "ReaderApp.hpp"

#include <atomic>
#include <csignal>
#include <functional>
#include <thread>

using namespace std::chrono_literals;

std::atomic_bool g_Running{true};
void Stop(int) { g_Running.store(false, std::memory_order_release); }

int main(int argc, char *argv[]) {
    std::signal(SIGINT, Stop);

    ReaderApp app(argc, argv);
    std::thread thread(&ReaderApp::Run, std::ref(app));

    // Wait for app to start running in thread
    while (!app.Running()) {
        std::this_thread::sleep_for(1s);
    }

    // Keep running until signaled or app stops
    while (g_Running && app.Running()) {
        std::this_thread::sleep_for(100ms);
    }

    app.Stop();
    thread.join();
}
