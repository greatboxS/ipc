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
                    std::cout << "args: <int> = " << *std::get<int*>(tp) << " <double> = " << std::get<double>(tp) << std::endl;
                }
            }
        } catch (...) {
        }
    }

    ipc::core::evloop::handle_s_ptr handle1;
    ipc::core::evloop::handle_s_ptr handle2;
};

bool task_handle(ipc::core::message_ptr mesg) {
    std::cout << "Task task_handle\n";
    if (mesg != nullptr) {
        std::cout << "message: " << mesg->data() << std::endl;
    }
    return false;
}

int main() {
    ipc::core::backtrace_init();
    classA a;

    ipc::core::shm_instance shm1("name1", 1024);

    if (shm1.create() != 0) {
        std::cout << "shm create failed\n";
        if (shm1.open() != 0) {
            std::cout << "shm open failed\n";
        } else {
            std::cout << "shm_open success\n";
        }
    } else {
        std::cout << "create shm success\n";
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

    std::mutex m1;
    ipc::core::worker_ptr wk = std::make_shared<ipc::core::worker>();
    int i = 0;
    for (i = 0; i < 10; i++) {
        wk->add_task(
            [&shm1, &m1, &msgqueue1](const int &a, const int &b, int &c, int d) {
                std::unique_lock<ipc::core::shm_instance> lock(shm1);
                tmp *tmp_ptr = shm1.get<tmp>();
                char buff[1024];

                if (msgqueue1.size() > 0) {
                    int bytes = msgqueue1.receive(buff, sizeof(buff));
                    if (bytes > 0) {
                        std::cout << "receive " << bytes << " from mesgqueue1\n";
                    }
                }
                msgqueue1.send("hello world", sizeof("hello world"));

                std::cout << "a = " << a << ", b = " << b << ", c = " << c << ", d = " << d << std::endl;
                std::cout << "tmp_ptr->a = " << tmp_ptr->a << "tmp_ptr->b = " << tmp_ptr->b << std::endl;
                std::cout << "\n";
                tmp_ptr->a++;
                tmp_ptr->b = tmp_ptr->a + 1;
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(500ms);
            },
            std::function<void()>(nullptr),
            (int)i, i, i, (int)i);
    }

    std::function<void(const char *, int &)> fnc = [](const char *str, int &a) {
        std::cout << "task std::function str" << str << ", a: " << a << std::endl;
    };

    int b = 0;
    wk->add_task(fnc, nullptr, "str", b);
    wk->add_task(task_handle, nullptr, ipc::core::message::create("", "", "hello task"));
    wk->start();
    wk->wait();
    bool done = wk->wait_for(1000);
    std::cout << "task done status: " << done << std::endl;
    // wk->quit();
    // shm1.destroy();

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

    ipc::core::message_ptr msg1 = ipc::core::message::create("sender1", "receiver1", "content1");
    ipc::core::evloop_man::get_instance().post_event(el1, std::move(msg1));

    ipc::core::message_ptr msg2 = ipc::core::message::create("sender2", "receiver2", "content2");
    ipc::core::evloop_man::get_instance().post_event(el2, std::move(msg2));

    for (i = 0; i < 100; i++) {
        ipc::core::evloop_man::get_instance().post_event(el2, std::string("sender ").append(std::to_string(i)), "receiver3", &i, 10.0);
    }

    try {
        ipc_throw_exception("Hello world %s, %d", "fjd", 10);
    } catch (std::exception &ex) {
        std::cout << ex.what() << std::endl;
    } catch (ipc::core::except &ex) {
        std::cout << ex.what() << std::endl;
    } catch (...) {
        std::cout << "other exception\n";
    }

    using namespace std::chrono_literals;

    // throw 1;

    std::this_thread::sleep_for(2000ms);

    return 0;
}
