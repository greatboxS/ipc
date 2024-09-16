#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/eventloop.h"
#include <iostream>
#include "concurrent/except.h"
#include "debuger/debuger.h"
#include "mutex/mutex_lock.h"
#include "unistd.h"
#include <thread>
#include "concurrent/condition_trigger.h"
#include "concurrent/task_chain.h"
#include "concurrent/callback.h"

static std::mutex mtx;
static std::mutex task_mtx;
static ipc::core::worker_ptr wk(new ipc::core::worker());
std::vector<std::future<void>> futures;
static ipc::core::condition_trigger trigger1;

using namespace std::chrono_literals;

class sequenctial_task : public ipc::core::task_chain {
public:
    ipc::core::trigger_ptr tg1, tg2, tg3, tg4;

    void init_task() {
        tg1 = add_task(ipc::core::make_task([]() {
                           printf("chain task 1: \n");
                           std::this_thread::sleep_for(100ms);
                       },
                                            nullptr),
                       ipc::core::make_trigger(1000));

        tg2 = add_task(ipc::core::make_task([]() {
                           printf("chain task 2: \n");
                           std::this_thread::sleep_for(100ms);
                       },
                                            nullptr),
                       ipc::core::make_trigger(1000));

        tg3 = add_task(ipc::core::make_task([]() {
                           printf("chain task 3: \n");
                           std::this_thread::sleep_for(100ms);
                       },
                                            nullptr),
                       ipc::core::make_trigger(1000));

        tg4 = add_task(ipc::core::make_task([]() {
                           printf("chain task 4: \n");
                           std::this_thread::sleep_for(100ms);
                       },
                                            nullptr),
                       ipc::core::make_trigger(1000));
    }

    void trigger1() { tg1->trigger(); }
    void trigger2() { tg2->trigger(); }
    void trigger3() { tg3->trigger(); }
    void trigger4() { tg4->trigger(); }

    sequenctial_task() :
        ipc::core::task_chain() {
    }
    virtual ~sequenctial_task() = default;

    virtual void on_task_completed() {
        printf("on_task_completed\n");
    }

    virtual void on_task_failed() {
        printf("on_task_failed\n");
    }

    virtual void on_task_timeout() {
        printf("on_task_timeout\n");
    }
};

int main() {
    try {
        wk->start();

        auto task_1 = wk->add_nocallback_task([]() {
            return 10;
        });

        std::thread thread11([]() {
            std::this_thread::sleep_for(100ms);
            trigger1.trigger();
        });
        thread11.detach();

        std::cout << "task_1 result = " << task_1->get()->data<int>(0) << std::endl;

        wk->add_nocallback_task([]() {
            trigger1.wait();
            printf("task_2 wait done!\n");
        });

        ipc::core::task_result result;
        result[1] = 42;
        printf("int [1] = %d\n", static_cast<int>(result[1]));

        result[2] = std::string("Hello, World!");
        printf("const char * [2] = %s\n", static_cast<const char *>(result[2]));

        result[3] = (double)100.0f;
        printf("double [3] = %f\n", (double)result[3]);

        result[4] = 192.0f;
        printf("float [4] = %f\n", (float)result[4]);

    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    } catch (ipc::core::except &ex) {
        std::cout << ex.what() << std::endl;
    } catch (...) {
        std::cout << "other exception\n";
    }

    auto chain = std::make_shared<sequenctial_task>();
    chain->init_task();
    wk->add_task(chain);

    std::thread thread22([chain]() {
        std::this_thread::sleep_for(1000ms);
        printf("tg1 trigger\n");
        chain->trigger1();

        std::this_thread::sleep_for(1000ms);
        printf("tg2 trigger\n");
        chain->trigger2();

        std::this_thread::sleep_for(1000ms);
        printf("tg3 trigger\n");
        chain->trigger3();

        std::this_thread::sleep_for(1000ms);
        printf("tg4 trigger\n");
        chain->trigger4();
    });
    thread22.detach();
    std::this_thread::sleep_for(4000ms);
    printf("chain state: %d\n", chain->state());

    ipc::core::callback<int, std::string> calback;

    for (int i = 0; i < 100; i++) {
        calback.register_callback(
            [](int value, const std::string &message) {
                std::cout << "Callback 1: Value = " << value << ", Message = " << message << std::endl;
            });
    }

    auto s = std::chrono::high_resolution_clock::now();
    calback.emit(42, "Hello World!");
    std::cout << "emit 1: " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - s).count() << std::endl;
    std::cout << "count :" << calback.count() << std::endl;

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
                        },
                                                         nullptr);

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