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
static std::mutex task_mtx;
static ipc::core::worker_ptr wk = nullptr;
std::vector<std::future<void>> futures;

int main() {

    wk = ipc::core::worker_man::get_instance().create_worker({}, true);
    wk->start();

    auto task_1 = wk->add_nocallback_task([]() {
        return 10;
    });

    std::cout << "task_1 result = " << task_1->get()->data<int>(0) << std::endl;

    ipc::core::task_result result;
    result[1] = 42;
    std::cout << "result[1] = " << static_cast<int>(result[1]) << std::endl;

    result[2] = std::string("Hello, World!");
    std::cout << "result[2] = " << static_cast<std::string&>(result[2]) << std::endl;

    using namespace std::chrono_literals;
    std::thread thread1([]() {
        int count = 0;
        while (++count < 2) {
            int time = 200;
            static std::vector<ipc::core::task_base_ptr> task_list;
            printf("++ start worker: %d, timeout: %d\n", wk->id(), time);
            futures.emplace_back(std::async(std::launch::async, [count, _wk(wk)]() {
                std::unique_lock<std::mutex> lock(mtx);
                for (int i = 0; i < 10000; i++) {
                    if (_wk->state() != static_cast<int>(ipc::core::worker::Exited)) {
                        auto task = ipc::core::make_task([count, i, _wk]() {
                            printf("==> worker: %d, task: %u\n", _wk->id(), i);
                            std::this_thread::sleep_for(1ms);
                        }, nullptr);

                        {
                            std::unique_lock<std::mutex> task_lock(task_mtx);
                            task_list.push_back(task);
                        }

                        wk->add_weak_task(task);
                    } else {
                        printf("==> worker: %d, executed count: %lu\n", _wk->id(), task_list.size());
                        break;
                    }
                }
            }));
            std::this_thread::sleep_for(200ms);
            printf("-- quit worker: %d, task_list: %ld, executed count: %lu\n", wk->id(), task_list.size(), wk->executed_count());
            {
                std::unique_lock<std::mutex> task_lock(task_mtx);
                task_list.clear();
            }
        }
        wk->quit();
        wk->detach();
    });

    thread1.join();
    return 0;
}