#include "worker_p.h"

#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#endif

namespace ipc::core {

static constexpr int WK_WAIT_TIMEOUT = 10000;

worker::impl::impl(const std::vector<task_base_ptr> &task_list, int id) :
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

worker::impl::~impl() {}

int worker::impl::id() const {
    return m_id;
}

int worker::impl::state() const {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    return static_cast<int>(m_state);
}

void worker::impl::start() {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_state != worker::Exited) {
        m_state = worker::Running;
    }
}

void worker::impl::stop() {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_state != worker::Exited) {
        m_state = worker::Stoped;
    }
}

void worker::impl::wait() {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_joined == false) {
        m_joined = true;
        lock.unlock();
        if (m_worker_thread.joinable() == true) {
            m_worker_thread.join();
        }
    }
}

void worker::impl::detach() {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_joined == false) {
        m_joined = true;
        m_worker_thread.detach();
    }
}

void worker::impl::quit() {
    {
        std::unique_lock<std::mutex> lock(m_task_queue_mtx);
        if (m_state != worker::Exited) {
            m_state = worker::Finalized;
        }
    }
    m_condition.notify_all();
}

size_t worker::impl::executed_count() const {
    return m_executed_count.load();
}

size_t worker::impl::task_count() const {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    return m_task_queue.size();
}

void worker::impl::assign_to(int cpu) {
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

void worker::impl::add_task(task_base_ptr task) {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_state != worker::Exited) {
        m_task_queue.emplace(std::move(task), task_base_weak_ptr{});
        m_condition.notify_one();
    }
}

void worker::impl::add_weak_task(task_base_weak_ptr task) {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_state != worker::Exited) {
        m_task_queue.emplace(task_base_ptr{nullptr}, std::move(task));
        m_condition.notify_one();
    }
}

void worker::impl::reset() {
    std::unique_lock<std::mutex> lock(m_task_queue_mtx);
    if (m_state != worker::Exited) {
        m_task_queue = {};
    }
}

std::thread::id worker::impl::thread_id() const {
    return m_worker_thread.get_id();
}

void worker::impl::run() {
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

} // namespace ipc::core
