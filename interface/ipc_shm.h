#ifndef IPC_SHM_H
#define IPC_SHM_H

#include "ipc_def.h"
#include "ipc_mutex.h"
#include <string>

namespace ipc {
class ipc_shm_private;
class ipc_shm {
public:
    explicit ipc_shm(const std::string &name);
    ~ipc_shm();

    ipc_shm(const ipc_shm &) = delete;
    ipc_shm &operator=(const ipc_shm &) = delete;

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

private:
    ipc_shm_private *m_priv = nullptr;
};

} // namespace ipc

#endif // IPC_SHM_H