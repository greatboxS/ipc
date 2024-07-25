#ifndef WORKER_H
#define WORKER_H

#include "task.h"
#include "task_helpers.h"
#include <memory>

namespace ipc::core {

class worker_base {
protected:
    virtual ~worker_base() = default;

public:
    virtual int state() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void quit() = 0;
    virtual size_t task_count() const = 0;
    virtual void wait() = 0;
    virtual bool wait_for(int ms) = 0;
    virtual void assign_to(int cpu) = 0;
};

class worker : public worker_base {
    worker(const worker &) = delete;
    worker &operator=(const worker &) = delete;

    enum worker_state {
        Idle,
        Running,
        Stoped,
        Finalized,
    };

    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

public:
    worker();
    virtual ~worker();

    template <typename F, typename... Args>
    auto add_task(F &&func, std::function<void()> callback, Args &&...args) {
        auto new_task = make_task(std::forward<F>(func), std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    template <typename R, typename... Args>
    auto add_task(R (*func)(Args...), std::function<void()> callback, Args &&...args) {
        auto new_task = make_task(func, std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    template <typename F, typename... Args>
    auto add_nocallback_task(F &&func, Args &&...args) {
        auto new_task = make_task(std::forward<F>(func), std::function<void()>(nullptr), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    int state() const override;
    void start() override;
    void stop() override;
    void wait() override;
    bool wait_for(int ms) override;
    void quit() override;
    size_t task_count() const override;
    void assign_to(int cpu) override;

private:
    void add_task(task_base_ptr task);
};

using worker_ptr = std::shared_ptr<worker>;
} // namespace ipc::core

#endif // WORKER_H