#ifndef WORKER_H
#define WORKER_H

#include "task.h"
#include "task_helpers.h"
#include <memory>
#include <vector>

namespace ipc::core {

class worker {
    worker(const worker &) = delete;
    worker(worker &&) = delete;
    worker &operator=(const worker &) = delete;

    class impl;
    std::unique_ptr<worker::impl> m_impl{nullptr};

public:
    enum State {
        Idle,
        Running,
        Stoped,
        Finalized,
        Exited,
    };

    worker(const std::vector<task_base_ptr> &task_list = {});
    ~worker();

    int id() const;
    int state() const;
    void start();
    void stop();
    void quit();
    void wait();
    void detach();
    size_t executed_count() const;
    size_t task_count() const;
    void assign_to(int cpu);
    void add_task(task_base_ptr task);
    void add_weak_task(task_base_weak_ptr task);
    void reset();
    std::thread::id thread_id() const;

    template <typename F, typename... Args>
    auto add_task(F func, std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
        auto new_task = make_task(std::move(func), std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    template <typename R, typename... Args>
    auto add_task(R (*func)(Args...), std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
        auto new_task = make_task(func, std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    template <typename F, typename... Args>
    auto add_nocallback_task(F func, Args &&...args) {
        auto new_task = make_task(std::move(func), std::function<void(ipc::core::task_base_ptr)>(nullptr), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }
};
using worker_ptr = std::shared_ptr<worker>;
} // namespace ipc::core

#endif // WORKER_H