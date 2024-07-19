#ifndef WORKER_H
#define WORKER_H

#include "task.h"
#include "task_helpers.h"
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <vector>

namespace ipc::core {

class worker_base {
protected:
    virtual ~worker_base() = default;

public:
    virtual int state() const = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void quit() = 0;
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

public:
    worker() :
        m_state(worker::Idle),
        m_finalized(false),
        m_worker_thread(new(std::nothrow) std::thread(&worker::run, this)),
        m_task_queue{},
        m_task_queue_mtx{},
        m_condition{} {
    }

    ~worker() {
        if (state() != static_cast<int>(worker::Finalized)) {
            quit();
        }
        if (m_worker_thread != nullptr) {
            if (m_worker_thread->joinable() == true) {
                m_worker_thread->join();
            }
            delete m_worker_thread;
            m_worker_thread = nullptr;
        }
    }

    template <typename F, typename... Args>
    auto add_task(F &&func, std::function<void()> callback, Args &&...args) {
        return std::move(add_task_handle(std::forward<F>(func), std::move(callback), std::forward<Args>(args)...));
    }

    template <typename F, typename... Args>
    auto add_nocallback_task(F &&func, Args &&...args) {
        return std::move(add_task_handle(std::forward<F>(func), std::function<void()>(nullptr), std::forward<Args>(args)...));
    }

    int state() const {
        return static_cast<int>(m_state.load());
    }

    void start() {
        m_state.store(worker::Running);
    }

    void stop() {
        m_state.store(worker::Stoped);
    }

    void quit() {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_finalized.store(true);
            m_state = worker::Finalized;
        }
        m_condition.notify_all();
    }

    void assign_to(int cpu) {
    }

private:
    template <typename F, typename... Args>
    auto add_task_handle(F &&func, std::function<void()> callback, Args &&...args) {
        auto new_task = make_task(std::forward<F>(func), std::move(callback), std::forward<Args>(args)...);
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_task_queue.push(new_task);
        }
        m_condition.notify_one();
        return std::move(new_task);
    }

    void run() {
        while (true) {
            task_base_ptr _task = {nullptr};
            {
                std::unique_lock<std::mutex> lock(m_task_queue_mtx);
                m_condition.wait(lock, [this] {
                    return ((m_finalized == true) || (m_task_queue.empty() == false));
                });

                if (m_state == worker::Running) {
                    _task = std::move(m_task_queue.front());
                    m_task_queue.pop();
                }
                if (m_finalized == true) {
                    m_task_queue = {};
                    break;
                }
            }
            if (_task != nullptr) {
                _task->execute();
            }
        }
    }

    std::atomic<worker_state> m_state = {worker::Idle};
    std::atomic<bool> m_finalized = {false};
    std::thread *m_worker_thread = nullptr;
    std::queue<task_base_ptr> m_task_queue = {};
    std::mutex m_task_queue_mtx = {};
    std::condition_variable m_condition = {};
};

using worker_ptr = std::shared_ptr<worker>;
} // namespace ipc::core

#endif // WORKER_H