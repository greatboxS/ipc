/**
 * @file csemaphore.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef CSEMAPHORE_H
#define CSEMAPHORE_H

#include "osal/osal.h"

namespace ipc::core {
class __dll_declspec__ csemaphore {
private:
    SEM_T m_stSem;

public:
    csemaphore();
    ~csemaphore();

    int create(int value, const char *name = NULL);
    int open(const char *name);
    int wait();
    int post();
    int value();
    int close();
    int destroy();
};
} // namespace ipc::core
#endif // CSEMAPHORE_H