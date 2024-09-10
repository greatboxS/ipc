#ifndef WORKER_P_H
#define WORKER_P_H

#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include "concurrent/worker.h"

namespace ipc::core {
class worker::impl {
    friend class worker_man;
    impl(const impl &) = delete;
    impl(impl &&) = delete;
    impl &operator=(const impl &) = delete;

public:
    impl(const std::vector<task_base_ptr> &task_list = {}, int id = -1);
    virtual ~impl();

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

private:
    void run();

    int m_id = 0;
    worker::State m_state = worker::Idle;
    std::queue<std::pair<task_base_ptr, task_base_weak_ptr>> m_task_queue = {};
    mutable std::mutex m_task_queue_mtx = {};
    std::condition_variable m_condition = {};
    bool m_joined = false;
    std::atomic<size_t> m_executed_count = {0};
    std::thread m_worker_thread = {};
};

} // namespace ipc::core

#endif // WORKER_P_H