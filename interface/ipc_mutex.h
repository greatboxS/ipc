#ifndef IPC_MUTEX_H
#define IPC_MUTEX_H

namespace ipc {

class ipc_locker {
protected:
    virtual ~ipc_locker() = default;

public:
    virtual void lock() = 0;
    virtual void unlock() = 0;

    virtual void try_lock() = 0;
    virtual void try_unlock() = 0;

    virtual void try_shared_lock() = 0;
    virtual void try_shared_unlock() = 0;
};

class ipc_mutex final : public ipc_locker {
public:
    void lock() override;
    void unlock() override;

    void try_lock() override;
    void try_unlock() override;

    void try_shared_lock() override;
    void try_shared_unlock() override;
};

} // namespace ipc

#endif // IPC_MUTEX_H