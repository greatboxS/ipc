#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/worker_manager.h"
#include "concurrent/eventloop_manager.h"
#include "concurrent/eventloop.h"
#include <iostream>
#include "exception/except.h"
#include "debuger/debuger.h"
#include "mutex/mutex_lock.h"
#include "unistd.h"
#include <thread>

static std::mutex mtx;
static ipc::core::worker_ptr wk = nullptr;
std::vector<std::future<void>> futures;

int main() {

    using namespace std::chrono_literals;
    std::thread thread1([]() {
        int count = 0;
        while (++count < 100) {
            int time = 200;
            wk = ipc::core::worker_man::get_instance().create_worker({}, true);
            wk->start();
            printf("++ start worker: %d, timeout: %d\n", wk->id(), time);
            futures.emplace_back(std::async(std::launch::async, [count, _wk(wk)]() {
                std::unique_lock<std::mutex> lock(mtx);
                for (int i = 0; i < 10000; i++) {
                    if (_wk->state() != static_cast<int>(ipc::core::worker::Exited)) {
                        _wk->add_task([count, i]() {
                            std::this_thread::sleep_for(1ms);
                        }, nullptr);
                    } else {
                        printf("==> worker: %d, executed count: %lu\n", _wk->id(), _wk->executed_count());
                        break;
                    }
                }
            }));
            std::this_thread::sleep_for(200ms);
            wk->quit();
            printf("-- quit worker: %d, executed count: %lu\n", wk->id(), wk->executed_count());
            wk->detach();
        }
    });

    thread1.join();
    return 0;
}