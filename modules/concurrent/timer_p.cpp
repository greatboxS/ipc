#include "concurrent/timer.h"
#include <mutex>

namespace ipc::core {
class timer::timer_p {
    friend class worker;
    int m_interval = 0;
    bool m_running = false;
    timer::timeout_callback m_callback{nullptr};
    std::shared_mutex m_mtx{};

    timer_p(int ms) :
        interval(m_interval),
        m_state(0),
        m_mtx{} {
    }
    ~timer_p() {
    }

    void set_callback(const timer::timeout_callback &callback) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_callback = callback;
    }

    void set_interval(int ms) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_interval = ms;
    }

    bool is_running() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_running;
    }

    void start() {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_running = true;
    }

    void stop() {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_running = false;
    }
};

timer::timer(int ms) :
    m_impl(std::unique_ptr<timer_p>(new timer_p(ms))) {
}
timer::~timer() {
}

void timer::set_callback(const timeout_callback &callback) {
    m_impl->set_callback(callback);
}
void timer::set_interval(int ms) {
    m_impl->set_interval(ms);
}
bool timer::is_running() const {
    return m_impl->is_running();
}
void timer::start() {
    m_impl->start();
}
void timer::stop() {
    m_impl->stop();
}
} // namespace ipc::core