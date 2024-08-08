#ifndef CMUTEX_H
#define CMUTEX_H

#include "osal/osal.h"

namespace ipc::core {
class __dll_declspec__ cmutex {
private:
    MUTEX_T m_stMtx;
    int32_t m_s32IsOpen = 0;

public:
    cmutex();
    ~cmutex();

    int create(const char *name = NULL, unsigned int recursive = 0);
    int lock(int timeout = 0);
    int try_lock();
    int unlock();
    int destroy();
};
} // namespace ipc::core
#endif // CMUTEX_H