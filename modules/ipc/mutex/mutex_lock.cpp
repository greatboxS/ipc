#include "mutex/mutex_lock.h"
#include "osac/cmutex.h"
#include "osac/csemaphore.h"
#include <string>

namespace ipc::core {
class local_mutex::impl : public cmutex {
    friend class mutex_lock;
    std::string m_name = "";
    int m_recursive = 0;

public:
    impl(const std::string &name, int recursive) :
        cmutex(),
        m_name(name),
        m_recursive(recursive) {
    }
    ~impl() {
    }

    void create() {
        (void)cmutex::create(m_name.data(), m_recursive);
    }

    void lock() {
        (void)cmutex::lock();
    }

    bool try_lock() {
        return (cmutex::try_lock() != -1);
    }

    void unlock() {
        (void)cmutex::unlock();
    }
};

/**
 * @fn local_mutex()
 * @brief Construct a new thread mutex::mutex lock object
 *
 */
local_mutex::local_mutex() :
    m_impl(std::make_unique<local_mutex::impl>(std::string(), 0)) {
    m_impl->create();
}

local_mutex::~local_mutex() {
    m_impl->destroy();
}

void local_mutex::lock() {
    m_impl->lock();
}

bool local_mutex::try_lock() {
    return m_impl->try_lock();
}

void local_mutex::unlock() {
    m_impl->unlock();
}

class global_mutex::impl : public csemaphore {
    friend class global_mutex;
    std::string m_name = "";

public:
    impl(const std::string &name) :
        csemaphore(),
        m_name(name) {
    }
    ~impl() {
    }

    void create() {
        if (csemaphore::create(1, m_name.data()) < 0) {
            csemaphore::open(m_name.data());
        }
    }

    void lock() {
        (void)csemaphore::wait();
    }

    bool try_lock() {
        return (csemaphore::wait() == 0);
    }

    void unlock() {
        (void)csemaphore::post();
    }
};
/**
 * @fn global_mutex(const std::string &name)
 * @brief Construct a new process mutex::process mutex object
 *
 * @param name
 */
global_mutex::global_mutex(const std::string &name) :
    m_impl(std::make_unique<global_mutex::impl>(name)) {
    m_impl->create();
}

global_mutex::~global_mutex() {
    m_impl->destroy();
}

void global_mutex::lock() {
    m_impl->lock();
}

bool global_mutex::try_lock() {
    return m_impl->try_lock();
}

void global_mutex::unlock() {
    m_impl->unlock();
}

} // namespace ipc::core
