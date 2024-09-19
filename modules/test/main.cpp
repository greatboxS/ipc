#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/eventloop.h"
#include <thread>
#include <iostream>
#include "concurrent/except.h"
#include "debuger/debuger.h"
#include "shm/shm_instance.h"
#include "message_queue/message_queue.h"
#include "mutex/mutex_lock.h"
#include "unistd.h"

class eventloop_1 : public ipc::core::evloop {
public:
    eventloop_1() :
        ipc::core::evloop(ipc::core::make_worker()),
        handle1(ipc::core::evloop::make_handle(std::bind(&eventloop_1::function1, this, std::placeholders::_1))),
        handle2(ipc::core::evloop::make_handle(std::bind(&eventloop_1::function2, this, std::placeholders::_1))) {
    }
    void function1(ipc::core::message_ptr x) {
        auto w = worker();
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

    void run(ipc::core::message_ptr mesg) override {
        function2(std::move(mesg));
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
    eventloop_1 ev1;
    ev1.start();

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

    ipc::core::worker_ptr wk = ipc::core::make_worker();
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

    ipc::core::message_args<int, int, std::string> args;
    args << 1 << 2 << "hello world\n";

    ipc::core::evloop_ptr el1 = ipc::core::make_evloop(ipc::core::make_worker());
    ipc::core::evloop_ptr el2 = ipc::core::make_evloop(ipc::core::make_worker());

    el1->start();
    el2->start();

    ipc::core::const_worker_ptr ev_worker = el1->get_worker();
    ev_worker->task_count();

    for (i = 0; i < 100; i++) {
        ev1.post(std::string("sender ").append(std::to_string(i)), "receiver3", &i, 10.0);
    }

    ipc::core::worker_ptr worker1 = ipc::core::make_worker({
        {ipc::core::make_task([]() {
            printf("initialize list task 1\n");
        },
                              nullptr)},
        {ipc::core::make_task([]() {
            printf("initialize list task 2\n");
        },
                              nullptr)},
    });
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(5000ms);

    worker1->start();
    worker1->quit();
    worker1->detach();
    wk->quit();
    wk->detach();
    el1->stop();
    el2->stop();
    shm1->close();
    ev1.stop();

    try {
        ipc_throw_exception("Hello world %s, %d", "fjd", 10);
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    } catch (...) {
        std::cout << "other exception\n";
    }
    return 0;
}
