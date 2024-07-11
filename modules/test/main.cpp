#include "mesg.h"
#include "worker_manager.h"
#include "eventloop_manager.h"
#include "eventloop.h"
#include <exception>
#include <thread>
#include "mesg.h"

int main() {
    ipc::core::worker_ptr worker_ptr = ipc::core::worker_man::get_instance().create();
    worker_ptr->add_task([]() {
    
    }, nullptr);

    auto el = ipc::core::evloop_man::get_instance().create_evloop();

    el->start();


    std::shared_ptr<ipc::core::message> msg = ipc::core::message::create("sender1", "receiver1", "content1");
    ipc::core::evloop_man::get_instance().post_event(el, std::move(msg));

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(2000ms);

    return 0;
}