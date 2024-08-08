#include "mutex/mutex_lock.h"
#include "osac/cmutex.h"
#include <string>
#include "mutex/mutex_lock.h"

namespace ipc::core {
class impl : public cmutex {
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
    m_impl(std::make_unique<impl>(std::string(), 0)) {
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

/**
 * @fn global_mutex(const std::string &name)
 * @brief Construct a new process mutex::process mutex object
 *
 * @param name
 */
global_mutex::global_mutex(const std::string &name) :
    m_impl(std::make_unique<impl>(name, 0U)) {
    m_impl->create();
}

global_mutex::~global_mutex() {
}

void global_mutex::destroy() {
    (void)m_impl->destroy();
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
