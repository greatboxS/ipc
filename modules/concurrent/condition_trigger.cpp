#include "concurrent/condition_trigger.h"
#include <condition_variable>
#include <mutex>
#include <atomic>

namespace ipc::core {
class condition_trigger::impl {
    friend class condition_trigger;
    int m_timeout = 0;
    bool m_done{false};
    std::mutex m_mtx{};
    std::condition_variable m_cv{};

public:
    impl(int ms) :
        m_timeout(ms),
        m_done(false),
        m_mtx{},
        m_cv{} {
    }
    void wait() {
        wait_for(m_timeout);
    }

    bool wait_for(int ms) {
        std::unique_lock<std::mutex> lock(m_mtx);
        if (m_done == false) {
            (void)m_cv.wait_for(lock, std::chrono::milliseconds(ms), [this]() {
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

condition_trigger::condition_trigger(int ms) :
    m_impl(new impl(ms)) {
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
