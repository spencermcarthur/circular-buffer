#include "WriterApp.hpp"

#include <atomic>
#include <csignal>
#include <functional>
#include <thread>

using namespace std::chrono_literals;

std::atomic_bool g_Running{true};
void Stop(int) { g_Running = false; }

int main(int argc, char *argv[]) {
    std::signal(SIGINT, Stop);

    WriterApp app(argc, argv);
    std::thread thread(&WriterApp::Run, std::ref(app));

    // Keep running until signaled
    while (g_Running) {
        std::this_thread::sleep_for(100ms);
    }

    app.Stop();
    thread.join();
}
