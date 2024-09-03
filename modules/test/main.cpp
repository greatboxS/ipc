#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/worker_manager.h"
#include "concurrent/eventloop_manager.h"
#include "concurrent/eventloop.h"
#include <exception>
#include <thread>
#include <iostream>
#include "exception/except.h"
#include "debuger/debuger.h"
#include "shm/shm_instance.h"
#include "message_queue/message_queue.h"
#include "mutex/mutex_lock.h"
#include "unistd.h"

class classA : public std::enable_shared_from_this<classA> {
public:
    classA() :
        handle1(ipc::core::evloop::make_handle(std::bind(&classA::function1, this, std::placeholders::_1))),
        handle2(ipc::core::evloop::make_handle(std::bind(&classA::function2, this, std::placeholders::_1))) {
    }
    void function1(ipc::core::message_ptr x) {
        std::cout << "function1: sender " << x->sender() << std::endl;
    }

    void function2(ipc::core::message_ptr x) {
        std::cout << "function2: sender " << x->sender() << std::endl;
        try {
            if (x != nullptr) {
                ipc::core::message_args<int *, double> args(x->data(), x->size());
                if (args.data().has_value()) {
                    auto tp = args.data().get();
                    std::cout << "args: <int> = " << *std::get<int *>(tp) << " <double> = " << std::get<double>(tp) << std::endl;
                }
            }
        } catch (...) {
        }
        using namespace std::chrono_literals;
        // std::this_thread::sleep_for(10ms);
    }

    ipc::core::evloop::handle_s_ptr handle1;
    ipc::core::evloop::handle_s_ptr handle2;
};

std::string task_handle(ipc::core::message_ptr mesg) {
    std::cout << "Task task_handle\n";
    if (mesg != nullptr) {
        std::cout << "message: " << mesg->data() << std::endl;
    }
    return "false";
}

ipc::core::shm_ptr shm1 = ipc::core::create_shm("name1", 1024);
std::mutex m1;
int i = 0;
ipc::core::global_mutex mutex1("mutex1");
ipc::core::local_mutex mutex2;

int main() {
    std::cerr << "+++++++++++++++++++++++++++++++++++++++++++++++++++++ " << getpid() << std::endl;
    ipc::core::backtrace_init();
    classA a;

    if (shm1->open() == false) {
        std::cout << "shm open failed\n";
    } else {
        std::cout << "shm_open success\n";
    }

    struct tmp {
        int a = 0;
        int b = 10;
    };

    ipc::core::message_queue msgqueue1("/msg1", 1000, 100);
    if (msgqueue1.create() == 0) {
        std::cout << "create message queue success\n";
    } else {
        std::cout << "create message queue failed, try open\n";
        if (msgqueue1.open() == 0) {
            std::cout << "open message queue success\n";

        } else {
            std::cout << "open message queue failed\n";
        }
    }

    ipc::core::worker_ptr wk = ipc::core::worker_man::get_instance().create_worker();
    for (i = 0; i < 100; i++) {
        wk->add_task(
            [&msgqueue1](const int &a, const int &b, int &c, int d, ipc::core::shm_ptr shm) {
                std::unique_lock<ipc::core::shm_instance> lock1(*shm);
                std::lock_guard<ipc::core::global_mutex> lock2(mutex1);
                std::lock_guard<ipc::core::local_mutex> lock3(mutex2);

                tmp *tmp_ptr = shm->get<tmp>();
                char buff[1024];

                if (msgqueue1.size() > 0) {
                    int bytes = msgqueue1.receive(buff, sizeof(buff));
                    if (bytes > 0) {
                        std::cout << "receive " << bytes << " from mesgqueue1\n";
                    }
                }
                msgqueue1.send("hello world", sizeof("hello world"));

                std::cout << "a = " << a << ", b = " << b << ", c = " << c << ", d = " << d << std::endl;
                std::cout << "tmp_ptr->a = " << tmp_ptr->a << ", tmp_ptr->b = " << tmp_ptr->b << std::endl;
                std::cout << "---------------- process: " << getpid() << ", thread: " << pthread_self() << "\n";
                tmp_ptr->a++;
                tmp_ptr->b = tmp_ptr->a + 1;
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1ms);
            },
            std::function<void(ipc::core::task_base_ptr)>(nullptr),
            (int)i, i, i, (int)i, shm1->shared_from_this());
    }

    std::function<void(const char *, const int &)> fnc = [](const char *str, const int &a) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
        std::cout << "task std::function str" << str << ", a: " << a << std::endl;
    };

    int b = 0;
    wk->add_task<decltype(fnc), const char *, const int &>(fnc, nullptr, "str111", b);
    wk->add_task(fnc, nullptr, "str222", (int)b);
    b = 10;

    wk->add_task(task_handle, [](ipc::core::task_base_ptr task) {
        auto result = task->get();
        if (result == nullptr) {
            return;
        }
        printf("result size: %lu, value: %s\n", result->size(), result->get<std::string>(0).value().c_str());
    }, ipc::core::message::create("", "", "hello task"));

    wk->start();
    bool done = ipc::core::worker_man::get_instance().wait(wk);
    std::cout << "==========================================wk================== " << done << " pid: " << getpid() << std::endl;

    ipc::core::message_args<int, int, std::string> args;
    args << 1 << 2 << "hello world\n";

    ipc::core::worker_ptr worker_ptr = ipc::core::worker_man::get_instance().create_worker();
    worker_ptr->add_task(
        []() {
        },
        nullptr);
    ipc::core::evloop_ptr el1 = ipc::core::evloop_man::get_instance().create_evloop(a.handle1);
    ipc::core::evloop_ptr el2 = ipc::core::evloop_man::get_instance().create_evloop(a.handle2);

    el1->start();
    el2->start();

    auto ev_worker = el1->get_worker();
    ev_worker->task_count();

    for (i = 0; i < 100; i++) {
        ipc::core::evloop_man::get_instance().post_event(el2, std::string("sender ").append(std::to_string(i)), "receiver3", &i, 10.0);
    }

    ipc::core::worker_ptr worker1 = ipc::core::worker_man::get_instance().create_worker({
        {ipc::core::make_task([]() {
            printf("initialize list task 1\n");
        },
                              nullptr)},
        {ipc::core::make_task([]() {
            printf("initialize list task 2\n");
        },
                              nullptr)},
    });

    worker1->start();

    done = ipc::core::worker_man::get_instance().wait(worker1);
    std::cout << "=====================================worker1======================= " << done << " pid: " << getpid() << std::endl;

    worker_ptr->start();
    done = ipc::core::worker_man::get_instance().wait(worker_ptr);
    std::cout << "=====================================worker_ptr======================= " << done << " pid: " << getpid() << std::endl;

    el1->stop();
    el2->stop();
    ipc::core::worker_man::get_instance().quit_all();
    shm1->close();

    try {
        ipc_throw_exception("Hello world %s, %d", "fjd", 10);
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    } catch (ipc::core::except &ex) {
        std::cout << ex.what() << std::endl;
    } catch (...) {
        std::cout << "other exception\n";
    }
    return 0;
}
