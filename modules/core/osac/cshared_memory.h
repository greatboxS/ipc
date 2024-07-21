#ifndef CSHARED_MEMORY_H
#define CSHARED_MEMORY_H

/**
 * @file cshared_memory.h
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
class __dll_declspec__ cshared_memory {
private:
    SHM_t m_stShm;

public:
    cshared_memory() {}
    ~cshared_memory() {}

    int open(const char *name, size_t size);
    int create(const char *name, size_t size);
    const SHM_t *get_shm() const { return &m_stShm; }
    int close();
    int release();

    int read(char *buff, size_t size);
    int write(char *buff, size_t size);
    int seek(int64_t pos, uint32_t type = 0);
    size_t size() const { return m_stShm.size; }
};
} // namespace ipc::core
#endif // CSHARED_MEMORY_H