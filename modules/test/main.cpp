#include "mesg.h"
#include "worker.h"

int main() {
    ipc::core::worker_ptr worker_ptr = ipc::core::worker_manager::create();
    worker_ptr->add_task([]() {
    
    });
    return 0;
}