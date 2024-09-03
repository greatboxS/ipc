#include "concurrent/mesg.h"
#include "concurrent/mesg_args.h"
#include "concurrent/worker_manager.h"
#include "concurrent/eventloop_manager.h"
#include "concurrent/eventloop.h"
#include <thread>
#include <iostream>
#include "concurrent/except.h"
#include "debuger/debuger.h"
#include "shm/shm_instance.h"
#include "message_queue/message_queue.h"
#include "mutex/mutex_lock.h"
#include "unistd.h"

int main(int argc, char const *argv[])
{
    printf("test_lib\n");
    return 0;
}