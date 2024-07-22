#ifndef CMESSAGE_QUEUE_H
#define CMESSAGE_QUEUE_H

/**
 * @file cmessage_queue.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "osal/osal.h"

namespace ipc::core {
class __dll_declspec__ cmessage_queue {
private:
    MSGQ_T m_stMsgq;

public:
    cmessage_queue() {}
    ~cmessage_queue() {}

    int create(const char *name, size_t msgsize, size_t msgcount);
    int destroy();
    int open(const char *name);
    int close();
    int size();
    int send(const char *buff, size_t size);
    int receive(char *buff, size_t size);
};
} // namespace ipc::core
#endif // CMESSAGE_QUEUE_H