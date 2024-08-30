#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <vector>
#include <iostream>
#include "worker_p.h"
#include "../identify/id_provider.h"

#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#endif

namespace ipc::core {

static constexpr int WK_WAIT_TIMEOUT = 10000;

class worker_p::impl {
    friend class worker_p;

    int m_id = 0;
    worker_state m_state = worker::Idle;
    std::queue<std::pair<task_base_ptr, task_base_weak_ptr>> m_task_queue = {};
    mutable std::mutex m_task_queue_mtx = {};
    std::condition_variable m_condition = {};
    bool m_joined = false;
    std::atomic<size_t> m_executed_count = {0};
    std::thread m_worker_thread = {};

public:
    impl(std::vector<task_base_ptr> task_list, int id) :
        m_id(id),
        m_state(worker::Idle),
        m_task_queue{},
        m_task_queue_mtx{},
        m_condition{},
        m_joined(false),
        m_executed_count(0),
        m_worker_thread(std::thread(&impl::run, this)) {
        for (auto task : task_list) {
            m_task_queue.emplace(std::move(task), task_base_weak_ptr{});
        }
    }

    ~impl() {}

    int id() const {
        return m_id;
    }

    int state() const {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        return static_cast<int>(m_state);
    }

    void start() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_state = worker::Running;
        }
    }

    void stop() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_state = worker::Stoped;
        }
    }

    void wait() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_joined == false) {
            m_joined = true;
            lock.unlock();
            if (m_worker_thread.joinable() == true) {
                m_worker_thread.join();
            }
        }
    }

    void detach() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_joined == false) {
            m_joined = true;
            m_worker_thread.detach();
        }
    }

    void quit() {
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            if (m_state != worker::Exited) {
                m_state = worker::Finalized;
            }
        }
        m_condition.notify_all();
    }

    size_t executed_count() const {
        return m_executed_count.load();
    }

    size_t task_count() const {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        return m_task_queue.size();
    }

    void assign_to(int cpu) {
#ifdef __linux__
        if (cpu >= 0) {
            cpu_set_t cpuset;
            int ret = 0;
            /* Set affinity mask to include CPUs (cpu) */
            CPU_ZERO(&cpuset);
            CPU_SET(cpu, &cpuset);

            pthread_t pthread = m_worker_thread.native_handle();
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

    void add_task(task_base_ptr task) {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_task_queue.emplace(std::move(task), task_base_weak_ptr{});
            m_condition.notify_one();
        }
    }

    void add_weak_task(task_base_weak_ptr task) {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_task_queue.emplace(task_base_ptr{nullptr}, std::move(task));
            m_condition.notify_one();
        }
    }

    void reset() {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_task_queue = {};
        }
    }

    int thread_id() {
        return m_worker_thread.native_handle();
    }

private:
    void run() {
        using namespace std::chrono_literals;
        do {
            task_base_ptr _task = {nullptr};
            try {
                {
                    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
                    if (m_task_queue.size() == 0) {
                        m_condition.wait_for(lock, std::chrono::milliseconds(1000), [this] {
                            return ((m_state != worker::Running) || (m_task_queue.empty() == false));
                        });
                    }
                    if (m_state == worker::Finalized) {
                        break;
                    } else if (m_state == worker::Running) {
                        if (m_task_queue.size() > 0) {
                            std::pair<task_base_ptr, task_base_weak_ptr> p = std::move(m_task_queue.front());
                            if (p.first != nullptr) {
                                _task = p.first;
                            } else {
                                _task = p.second.lock();
                            }
                            m_task_queue.pop();
                        }
                    } else {
                        std::this_thread::sleep_for(1ms);
                    }
                }

                if (_task != nullptr) {
                    _task->execute();
                    m_executed_count.fetch_add(1U);
                }
            } catch (...) {
                // Do nothing
            }
        } while (m_state != worker::Finalized);
        {
            std::unique_lock<std::mutex> lock(m_task_queue_mtx);
            m_state = worker::Exited;
        }
    }
};

/**
 * @fn worker_p::worker_p()
 * @brief Construct a new worker_p::worker_p object
 *
 */
worker_p::worker_p(std::vector<task_base_ptr> task_list) :
    m_impl(std::make_unique<worker_p::impl>(task_list, get_new_id<id_provider_type::Worker>())) {
}

worker_p::~worker_p() {
}
int worker_p::id() const {
    return m_impl->id();
}

int worker_p::state() const {
    return m_impl->state();
}
void worker_p::start() {
    m_impl->start();
}
void worker_p::stop() {
    m_impl->stop();
}
void worker_p::quit() {
    m_impl->quit();
}
void worker_p::wait() {
    m_impl->wait();
}
void worker_p::detach() {
    m_impl->detach();
}
size_t worker_p::executed_count() const {
    return m_impl->executed_count();
}
size_t worker_p::task_count() const {
    return m_impl->task_count();
}
void worker_p::assign_to(int cpu) {
    m_impl->assign_to(cpu);
}
void worker_p::add_task(task_base_ptr task) {
    m_impl->add_task(task);
}
void worker_p::add_weak_task(task_base_weak_ptr task) {
    m_impl->add_weak_task(task);
}
void worker_p::reset() {
    m_impl->reset();
}
int worker_p::thread_id() const {
    return m_impl->thread_id();
}
} // namespace ipc::core
