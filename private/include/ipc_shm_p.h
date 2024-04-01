#ifndef IPC_SHM_P_H
#define IPC_SHM_P_H

#include "interface/ipc_shm.h"

namespace ipc {

class ipc_shm_private {
public:
    void lock();
    void unlock();

    void try_lock();
    void try_unlock();

    void try_shared_lock();
    void try_shared_unlock();

    size_t size() const;
    void read(uint8_t *data, size_t size);
    void read(char *data, size_t size);
    void write(const uint8_t *data, size_t size);
    void write(const char *data, size_t size);
    void write(const std::string &data);
};
} // namespace ipc

#endif // IPC_SHM_P_H