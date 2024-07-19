#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/worker_manager.h"
#include "concurrent/eventloop_manager.h"
#include "concurrent/eventloop.h"
#include <exception>
#include <thread>
#include <iostream>

class classA : public std::enable_shared_from_this<classA> {
public:
    classA() :
        handle1(ipc::core::evloop::make_handle(std::bind(&classA::function1, this, std::placeholders::_1))),
        handle2(ipc::core::evloop::make_handle(std::bind(&classA::function2, this, std::placeholders::_1))) {
    }
    void function1(ipc::core::message_ptr x) {
        // std::cout << "function1 is running with x = " << x << "\n";
    }

    void function2(ipc::core::message_ptr x) {
        // std::cout << "function2 is running with x = " << x << "\n";
        printf("function2\n");

        if (x->sender() == "sender3") {
            std::cout << "parse from sender 3\n";

            if (x != nullptr) {
                ipc::core::message_args<int, double> args(x->data(), x->size());
                if (args.data().has_value()) {
                    auto tp = args.data().get();
                    std::cout << "args: <int> = " << std::get<int>(tp) << " <double> = " << std::get<double>(tp) << std::endl;
                }
            }
        }
    }

    ipc::core::evloop::handle_s_ptr handle1;
    ipc::core::evloop::handle_s_ptr handle2;
};

void task_handle(ipc::core::message_ptr mesg) {
    std::cout << "Task task_handle\n";
}

int main() {

    ipc::core::message_args<int, int, std::string> args;
    args << 1 << 2 << "hello world";

    ipc::core::worker_ptr worker_ptr = ipc::core::worker_man::get_instance().create();
    worker_ptr->add_task([]() {

    },
                         nullptr);
    classA a;
    ipc::core::evloop_ptr el1 = ipc::core::evloop_man::get_instance().create_evloop(a.handle1);
    ipc::core::evloop_ptr el2 = ipc::core::evloop_man::get_instance().create_evloop(a.handle2);

    el1->start();
    el2->start();

    ipc::core::message_ptr msg1 = ipc::core::message::create("sender1", "receiver1", "content1");
    ipc::core::evloop_man::get_instance().post_event(el1, std::move(msg1));

    ipc::core::message_ptr msg2 = ipc::core::message::create("sender2", "receiver2", "content2");
    ipc::core::evloop_man::get_instance().post_event(el2, std::move(msg2));

    ipc::core::evloop_man::get_instance().post_event(el2, "sender3", "receiver3", 10, 100.0);

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);

    return 0;
}