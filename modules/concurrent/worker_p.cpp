#include "concurrent/worker.h"
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <vector>

namespace ipc::core {

class worker::impl {
    friend class worker;

    std::atomic<worker_state> m_state = {worker::Idle};
    std::atomic<bool> m_finalized = {false};
    std::thread *m_worker_thread = nullptr;
    std::queue<task_base_ptr> m_task_queue = {};
    mutable std::mutex m_task_queue_mtx = {};
    std::condition_variable m_condition = {};
    std::atomic<bool> m_done = {false};
    std::mutex m_done_mtx = {};
    std::condition_variable m_done_condition = {};

public:
    impl() :
        m_state(worker::Idle),
        m_finalized(false),
        m_worker_thread(new(std::nothrow) std::thread(&impl::run, this)),
        m_task_queue{},
        m_task_queue_mtx{},
        m_condition{},
        m_done(false),
        m_done_mtx{},
        m_done_condition{} {
    }

    ~impl() {
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

    int state() const {
        return static_cast<int>(m_state.load());
    }

    void start() {
        m_done.store(false);
        m_state.store(worker::Running);
    }

    void stop() {
        m_state.store(worker::Stoped);
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_done_mtx);
        m_done_condition.wait(lock, [this] {
            return (m_done.load() == true);
        });
    }

    bool wait_for(int ms) {
        std::unique_lock<std::mutex> lock(m_done_mtx);
        bool done = m_done_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] {
            return (m_done.load() == true);
        });
        return done;
    }

    void quit() {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_finalized.store(true);
            m_state = worker::Finalized;
        }
        m_condition.notify_all();
    }

    size_t task_count() const {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        return m_task_queue.size();
    }

    void assign_to(int cpu) {
    }

    void add_task(task_base_ptr task) {
        m_done.store(false);
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_task_queue.push(task);
        }
        m_condition.notify_one();
    }

private:
    void run() {
        while (true) {
            task_base_ptr _task = {nullptr};
            bool done = false;
            {
                std::unique_lock<std::mutex> lock(m_task_queue_mtx);
                m_condition.wait(lock, [this] {
                    return ((m_finalized == true) || (m_task_queue.empty() == false));
                });

                if (m_state == worker::Running) {
                    _task = std::move(m_task_queue.front());
                    m_task_queue.pop();
                }

                if (m_task_queue.empty() == true) {
                    std::unique_lock<std::mutex> lock(m_done_mtx);
                    m_done.store(true);
                    m_done_condition.notify_all();
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
};

/**
 * @fn worker::worker()
 * @brief Construct a new worker::worker object
 *
 */
worker::worker() :
    m_impl(std::make_unique<worker::impl>()) {
}

worker::~worker() {
}

int worker::state() const {
    return m_impl->state();
}
void worker::start() {
    m_impl->start();
}
void worker::stop() {
    m_impl->stop();
}
void worker::wait() {
    m_impl->wait();
}
bool worker::wait_for(int ms) {
    return m_impl->wait_for(ms);
}
void worker::quit() {
    m_impl->quit();
}
size_t worker::task_count() const {
    return m_impl->task_count();
}
void worker::assign_to(int cpu) {
    m_impl->assign_to(cpu);
}
void worker::add_task(task_base_ptr task) {
    m_impl->add_task(task);
}
} // namespace ipc::core
