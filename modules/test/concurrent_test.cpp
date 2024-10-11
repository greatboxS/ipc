#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/eventloop.h"
#include "concurrent/except.h"
#include "debuger/debuger.h"
#include "mutex/mutex_lock.h"
#include "concurrent/condition_trigger.h"
#include "concurrent/task_chain.h"
#include "concurrent/callback.h"
#include <gtest/gtest.h>
#include <future>
#include <thread>

using namespace ipc::core;
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

class TaskChainTest : public ::testing::Test {
protected:
    worker_ptr wk;

    void SetUp() override {
        wk = std::make_shared<worker>();
        wk->start();
    }

    void TearDown() override {
        if (wk->state() != worker::Exited) {
            wk->quit();
            wk->detach();
        }
    }
};

// Test sequential task chain
TEST_F(TaskChainTest, SequentialTaskExecution) {
    auto chain = std::make_shared<sequenctial_task>();
    chain->init_task();
    wk->add_task(chain);

    std::thread task([this, chain]() {
        chain->execute();
    });
    task.detach();

    std::thread trigger_thread([chain]() {
        std::this_thread::sleep_for(100ms);
        chain->trigger1();

        std::this_thread::sleep_for(100ms);
        chain->trigger2();

        std::this_thread::sleep_for(100ms);
        chain->trigger3();

        std::this_thread::sleep_for(100ms);
        chain->trigger4();
    });
    trigger_thread.join();

    EXPECT_EQ(chain->state(), static_cast<int>(ipc::core::task_base::state::Finished));
}

// Test condition trigger
// TEST_F(TaskChainTest, ConditionTrigger) {
//     condition_trigger trigger;

//     auto task = wk->add_nocallback_task([&trigger]() {
//         trigger.wait();
//         return 42;
//     });

//     std::thread trigger_thread([&trigger]() {
//         std::this_thread::sleep_for(200ms);
//         trigger.trigger();
//     });
//     trigger_thread.join();

//     EXPECT_EQ(task->get()->data<int>(0), 42);
// }

// Test task result with various data types
TEST(TaskResultTest, TaskResultDataStorage) {
    task_result result;
    result[1] = 123;
    result[2] = std::string("Hello, World!");
    result[3] = 3.14;
    result[4] = 42.0f;

    EXPECT_EQ(static_cast<int>(result[1]), 123);
    EXPECT_STREQ(static_cast<const char *>(result[2]), "Hello, World!");
    EXPECT_DOUBLE_EQ(static_cast<double>(result[3]), 3.14);
    EXPECT_FLOAT_EQ(static_cast<float>(result[4]), 42.0f);
}

// Test callback registration and execution
TEST_F(TaskChainTest, CallbackRegistration) {
    callback<int, std::string> cb;

    bool callback_invoked = false;
    auto connect = cb.register_callback([&callback_invoked](int value, const std::string &message) {
        EXPECT_EQ(value, 42);
        EXPECT_EQ(message, "Hello World!");
        callback_invoked = true;
    });

    cb(42, "Hello World!");
    EXPECT_TRUE(callback_invoked);
    EXPECT_EQ(cb.count(), 1);
}

// Test worker handling multiple tasks and quitting
TEST_F(TaskChainTest, WorkerHandlesMultipleTasksAndQuits) {
    auto wk = std::make_shared<worker>();
    wk->start();
    std::vector<ipc::core::task_base_ptr> tasks;

    for (int i = 0; i < 1000; ++i) {
        tasks.emplace_back(wk->add_nocallback_task([i]() {
            std::this_thread::sleep_for(1ms);
            return i * 2;
        }));
    }

    for (auto &t : tasks) {
        t->get();
    }

    wk->quit();
    wk->join();
    EXPECT_EQ(wk->state(), worker::Exited);
}

// Test task timeout
TEST_F(TaskChainTest, TaskTimeout) {
    auto task = wk->add_nocallback_task([]() {
        std::this_thread::sleep_for(500ms);
        return 5;
    });

    EXPECT_EQ(task->get(100), nullptr); // Should timeout
    std::this_thread::sleep_for(500ms); // Wait for task to complete
}

// Test task_chain completion
TEST_F(TaskChainTest, TaskChainCompletionCallback) {
    auto chain = std::make_shared<sequenctial_task>();
    chain->init_task();

    bool completed = false;
    chain->set_handle([&](int) { completed = true; });

    wk->add_task(chain);
    std::thread trigger_thread([chain]() {
        std::this_thread::sleep_for(100ms);
        chain->trigger1();
        std::this_thread::sleep_for(100ms);
        chain->trigger2();
        std::this_thread::sleep_for(100ms);
        chain->trigger3();
        std::this_thread::sleep_for(100ms);
        chain->trigger4();
    });
    trigger_thread.join();

    EXPECT_TRUE(completed);
}

// Test invalid trigger activation (triggering out of sequence)
TEST_F(TaskChainTest, InvalidTriggerActivation) {
    auto chain = std::make_shared<sequenctial_task>();
    chain->init_task();
    wk->add_task(chain);

    chain->trigger2();  // Trigger task 2 before task 1 (invalid case)

    std::this_thread::sleep_for(2000ms);
    EXPECT_EQ(chain->state(), static_cast<int>(ipc::core::task_base::state::Timeout));  // Task chain should be in failed state
}

// Test task failure handling
TEST_F(TaskChainTest, TaskFailureHandling) {
    auto failing_task = std::make_shared<sequenctial_task>();

    // Override a task to throw an exception
    failing_task->tg1 = failing_task->add_task(make_task([]() {
        printf("Simulated task\n");
        throw std::runtime_error("Simulated failure");
        printf("After throw\n");
    }, nullptr), make_trigger(1000));

    bool failed = false;
    failing_task->set_handle([&](int state) {
        printf("on task state: %d\n", state);
        failed = (state != static_cast<int>(ipc::core::task_base::state::Finished)); 
    });

    wk->add_task(failing_task);
    // failing_task->trigger1();

    std::this_thread::sleep_for(100ms);
    EXPECT_TRUE(failed);
}

// Test handling of a high volume of tasks
TEST_F(TaskChainTest, HighVolumeTaskExecution) {
    auto wk = std::make_shared<worker>();
    wk->start();
    std::vector<ipc::core::task_base_ptr> tasks;
    for (int i = 0; i < 10000; ++i) {
        tasks.emplace_back(wk->add_nocallback_task([i]() {
            std::this_thread::sleep_for(1ms);
        }));
    }

    for (auto& t : tasks) {
        t->get();
    }

    EXPECT_EQ(wk->executed_count(), 10000);
    wk->quit();
    wk->detach();
}

// Test task interruption and worker shutdown during task execution
TEST_F(TaskChainTest, TaskInterruption) {
    auto wk = std::make_shared<worker>();
    wk->start();
    auto task = wk->add_nocallback_task([]() {
        std::this_thread::sleep_for(500ms);
        return 5;
    });

    std::this_thread::sleep_for(200ms);
    wk->quit();  // Interrupt the worker before the task finishes
    wk->join();

    EXPECT_EQ(wk->state(), worker::Exited);
}

// Test multiple task chains executed simultaneously
TEST_F(TaskChainTest, MultipleTaskChains) {
    auto chain1 = std::make_shared<sequenctial_task>();
    auto chain2 = std::make_shared<sequenctial_task>();

    chain1->init_task();
    chain2->init_task();

    wk->add_task(chain1);
    wk->add_task(chain2);

    std::thread t1([chain1]() {
        std::this_thread::sleep_for(100ms);
        chain1->trigger1();
    });

    std::thread t2([chain2]() {
        std::this_thread::sleep_for(200ms);
        chain2->trigger1();
    });

    t1.join();
    t2.join();

    EXPECT_EQ(chain1->state(), static_cast<int>(ipc::core::task_base::state::Executing));
    EXPECT_EQ(chain2->state(), static_cast<int>(ipc::core::task_base::state::Executing));
}

// Test worker restart after quitting
TEST_F(TaskChainTest, WorkerRestartAfterQuit) {
    wk->stop();
    EXPECT_EQ(wk->state(), worker::Stopped);

    wk->start();  // Restart the worker
    EXPECT_EQ(wk->state(), worker::Running);

    auto task = wk->add_nocallback_task([]() { return 42; });
    EXPECT_EQ(task->get()->data<int>(0), 42);
}

// Test task timeout handling
TEST_F(TaskChainTest, TaskTimeoutHandling) {
    auto chain = std::make_shared<sequenctial_task>();

    // Set a very short timeout for task 1
    chain->tg1 = chain->add_task(make_task([]() {
        std::this_thread::sleep_for(200ms);  // Task will exceed the timeout
    }, nullptr), make_trigger(100));

    bool finished = false;
    chain->set_handle([&](int state) { finished = (state == static_cast<int>(ipc::core::task_base::state::Finished)); });

    wk->add_task(chain);
    chain->trigger1();

    std::this_thread::sleep_for(300ms);
    EXPECT_TRUE(finished);
}

// Test concurrent task execution from multiple threads
TEST_F(TaskChainTest, ConcurrentTaskExecutionFromMultipleThreads) {
    std::vector<std::thread> threads;
    auto wk = std::make_shared<worker>();
    wk->start();
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([this, i, wk]() {
            for (int j = 0; j < 1000; ++j) {
                wk->add_nocallback_task([i, j]() {
                    std::this_thread::sleep_for(1ms);
                });
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    std::this_thread::sleep_for(12000ms);
    EXPECT_EQ(wk->executed_count(), 10000);
    wk->quit();
    wk->detach();
}