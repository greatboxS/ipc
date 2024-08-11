#ifndef CSECURED_SHARED_MEM_H
#define CSECURED_SHARED_MEM_H

#include "osal/osal.h"
#include <string>

namespace ipc::core {
class __dll_declspec__ csecured_shared_mem {
private:
    class impl;
    impl *m_impl = nullptr;

public:
    explicit csecured_shared_mem(const std::string &name, size_t size);
    ~csecured_shared_mem();

    int create();
    int destroy();
    int open();
    int opened() const;

    void lock();
    void unlock();
    bool try_lock();

    void *get_base_addr();
    size_t size() const;

    int read(char *buff, size_t size);
    int write(char *buff, size_t size);
    int64_t seek(int64_t pos, uint32_t type);
    int64_t current_pos();
};

} // namespace ipc::core

#endif // CSECURED_SHARED_MEM_H