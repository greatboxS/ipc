#include "concurrent/worker.h"
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <vector>
#include <iostream>

#ifdef __linux__
#include <pthread.h>
#endif

namespace ipc::core {

class worker::impl {
    friend class worker;

    worker_state m_state = worker::Idle;
    std::thread *m_worker_thread = nullptr;
    std::queue<task_base_ptr> m_task_queue = {};
    mutable std::mutex m_task_queue_mtx = {};
    std::condition_variable m_condition = {};
    bool m_done = false;
    std::mutex m_done_mtx = {};
    std::condition_variable m_done_condition = {};

public:
    impl(std::initializer_list<task_base_ptr> task_list) :
        m_state(worker::Idle),
        m_worker_thread(new(std::nothrow) std::thread(&impl::run, this)),
        m_task_queue{task_list},
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
        return static_cast<int>(m_state);
    }

    void start() {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_state = worker::Running;
        }
        set_done_status(false);
    }

    void stop() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        m_state = worker::Stoped;
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_done_mtx);
        m_done_condition.wait(lock, [this] {
            return (m_done == true);
        });
    }

    bool wait_for(int ms) {
        std::unique_lock<std::mutex> lock(m_done_mtx);
        bool done = m_done_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] {
            return (m_done == true);
        });
        return done;
    }

    void quit() {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_state = worker::Finalized;
        }
        m_condition.notify_all();
    }

    size_t task_count() const {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        return m_task_queue.size();
    }

    void assign_to(int cpu) {
        if (m_worker_thread != nullptr) {
#ifdef __linux__
            if (cpu >= 0) {
                cpu_set_t cpuset;
                int ret = 0;
                /* Set affinity mask to include CPUs (cpu) */
                CPU_ZERO(&cpuset);
                CPU_SET(cpu, &cpuset);

                pthread_t pthread = m_worker_thread->native_handle();
                ret = pthread_setaffinity_np(pthread, sizeof(cpuset), &cpuset);
                if (ret != 0) {
                    fprintf(stderr, "[%s] pthread_setaffinity_np failed, %d\n", __FUNCTION__, ret);
                }
                /* Check the actual affinity mask assigned to the pthread. */
                ret = pthread_getaffinity_np(pthread, sizeof(cpuset), &cpuset);
                if (ret != 0) {
                    fprintf(stderr, "[%s] pthread_getaffinity_np failed, %d\n", __FUNCTION__, ret);
                }

                for (int j = 0; j < CPU_SETSIZE; j++) {
                    CPU_ISSET(j, &cpuset);
                }
            }
#endif
        }
    }

    void add_task(task_base_ptr task) {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_task_queue.push(task);
        }
        m_condition.notify_one();
    }

    void set_done_status(bool done) {
        std::unique_lock<std::mutex> lock(m_done_mtx);
        m_done = done;
    }

private:
    void run() {
        bool done = false;
        while (true) {
            task_base_ptr _task = {nullptr};
            try {
                {
                    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
                    m_condition.wait_for(lock, std::chrono::milliseconds(1000), [this] {
                        return ((m_state == worker::Finalized) || (m_task_queue.empty() == false));
                    });

                    if (m_state == worker::Finalized) {
                        m_task_queue = {};
                        break;
                    }
                    if (m_state == worker::Running) {
                        if (m_task_queue.size() > 0) {
                            _task = std::move(m_task_queue.front());
                            m_task_queue.pop();
                        }

                        if (m_task_queue.size() == 0) {
                            done = true;
                        }
                    }
                }
                if (_task != nullptr) {
                    _task->execute();
                }

                if (done == true) {
                    done == false;
                    set_done_status(true);
                    m_done_condition.notify_all();
                }
            } catch (...) {
                // Do nothing
            }
        }

        set_done_status(true);
        m_done_condition.notify_all();
    }
};

/**
 * @fn worker::worker()
 * @brief Construct a new worker::worker object
 *
 */
worker::worker(std::initializer_list<task_base_ptr> task_list) :
    m_impl(std::make_unique<worker::impl>(task_list)) {
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
