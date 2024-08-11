#ifndef MUTEX_LOCK_H
#define MUTEX_LOCK_H

#include <memory>

namespace ipc::core {

/**
 * @fn in process thread mutex
 * @brief
 *
 */
class local_mutex {
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};
    local_mutex(const local_mutex &) = delete;
    local_mutex &operator=(const local_mutex &) = delete;

public:
    local_mutex();
    ~local_mutex();

    void lock();
    bool try_lock();
    void unlock();
};

/**
 * @fn cross process thread mutex
 * @brief
 *
 */
class global_mutex {
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};
    global_mutex(const global_mutex &) = delete;
    global_mutex &operator=(const global_mutex &) = delete;

public:
    global_mutex(const std::string &name);
    ~global_mutex();

    void lock();
    bool try_lock();
    void unlock();
};

} // namespace ipc::core

#endif // MUTEX_LOCK_H