#include "concurrent/condition_trigger.h"
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace ipc::core {
class condition_trigger::impl {
    friend class condition_trigger;
    bool m_done{false};
    std::mutex m_mtx{};
    std::condition_variable m_cv{};

public:
    void wait() {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_done == false) {
            m_cv.wait(lock, [this]() {
                return (m_done == true);
            });
        }
    }

    bool wait_for(int ms) {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_done == false) {
            (void)m_cv.wait_for(lock, std::chrono::milliseconds(1000), [this]() {
                return (m_done == true);
            });
        }
        return triggered();
    }

    bool triggered() const {
        return m_done;
    }

    void trigger() {
        set_status(true);
        m_cv.notify_all();
    }

    void set_status(bool status) {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_done = status;
    }
};

condition_trigger::condition_trigger() :
    m_impl(new impl()) {
}

condition_trigger::~condition_trigger() {
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void condition_trigger::wait() {
    m_impl->wait();
}

bool condition_trigger::wait_for(int ms) {
    return m_impl->wait_for(ms);
}

bool condition_trigger::triggered() const {
    return m_impl->triggered();
}

void condition_trigger::trigger() {
    m_impl->trigger();
}

void condition_trigger::reset() {
    if (m_impl->triggered() == true) {
        m_impl->set_status(false);
    }
}

} // namespace ipc::core
