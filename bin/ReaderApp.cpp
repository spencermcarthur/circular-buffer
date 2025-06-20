#include "ReaderApp.hpp"

#include <atomic>
#include <csignal>
#include <functional>
#include <thread>

using namespace std::chrono_literals;

std::atomic_bool g_Running{true};
void stop(int) { g_Running.store(false, std::memory_order_release); }

int main(int argc, char *argv[]) {
    std::signal(SIGINT, stop);

    ReaderApp app(argc, argv);
    std::thread thread(&ReaderApp::Run, std::ref(app));

    while (g_Running && app.Running()) {
        std::this_thread::sleep_for(1s);
    }

    app.Stop();
    thread.join();
}
