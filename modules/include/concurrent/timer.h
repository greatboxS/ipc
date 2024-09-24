#ifndef CONCURRENT_TIMER_H
#define CONCURRENT_TIMER_H

#include <functional>
#include <memory>

class worker;

namespace ipc::core {
class timer {
    timer(const timer &) = delete;
    timer(timer &&) = delete;
    timer &operator=(const timer &) = delete;

    class timer_p;
    std::unique_ptr<timer_p> m_impl{nullptr};
    timer(int ms);
    ~timer();

public:
    using timeout_callback = std::function<void()>;

    void set_callback(const timeout_callback &callback);
    void set_interval(int ms);
    bool is_running() const;
    void start();
    void stop();
};
} // namespace ipc::core

#endif // CONCURRENT_TIMER_H