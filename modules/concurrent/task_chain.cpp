#include "concurrent/task_chain.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ipc::core {

class task_chain::impl {
    friend class task_chain;
    struct task_group {
        task_base_ptr task;
        trigger_ptr trigger;
    };
    std::queue<std::pair<task_base_ptr, trigger_ptr>> queue{};
    std::atomic<int> state = static_cast<int>(task_base::state::Created);
    std::atomic<int> executed = 0;
    std::exception_ptr exception{nullptr};
    std::function<void(int)> callback{nullptr};
    bool finished = false;
    std::mutex mtx{};
    task_result result{};
    std::condition_variable condition{};
};

task_chain::task_chain() :
    m_impl(new task_chain::impl()) {
}

task_chain::~task_chain() {
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void task_chain::execute() {
    auto &que = m_impl->queue;
    m_impl->result.clear();
    int index = 0;
    m_impl->exception = nullptr;
    try {
        m_impl->state.store(static_cast<int>(task_base::state::Executing));
        if (m_impl->callback != nullptr) {
            m_impl->callback(m_impl->state);
        }
        while (que.empty() == false) {
            auto p = que.front();
            que.pop();
            auto &task = p.first;
            auto &trigger = p.second;

            if (task != nullptr) {
                /* Execute task */
                task->execute();
                m_impl->result[index] = task->get();
                index += 1;

                if (trigger == nullptr) {
                    m_impl->executed.fetch_add(1);
                    continue;
                }

                /* Wait for a task to be trigger for next task to be executed*/
                trigger->wait();
                if (trigger->triggered() == false) {
                    m_impl->state.store(static_cast<int>(task_base::state::Timeout));
                    on_task_timeout();
                    break;
                } else {
                    m_impl->executed.fetch_add(1);
                }
            }
        }

        /* Setting task state to Finished */
        if (m_impl->state.load() != static_cast<int>(task_base::state::Timeout)) {
            m_impl->state.store(static_cast<int>(task_base::state::Finished));
            on_task_completed();
        }
    } catch (...) {
        m_impl->exception = std::current_exception();
        m_impl->state.store(static_cast<int>(task_base::state::Failed));
        on_task_failed();
    }

    {
        std::unique_lock<std::mutex> lock(m_impl->mtx);
        m_impl->finished = true;
    }
    if (m_impl->callback != nullptr) {
        m_impl->callback(m_impl->state);
    }

    if (m_impl->exception != nullptr) {
        throw m_impl->exception;
    }
}

std::exception_ptr task_chain::exception_ptr() const {
    return m_impl->exception;
}

const task_result *task_chain::get(int ms) {
    task_result *_task_result = nullptr;
    std::unique_lock<std::mutex> lock(m_impl->mtx);
    bool done = m_impl->condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return m_impl->finished; });
    if (done == true) {
        _task_result = &m_impl->result;
    }
    return _task_result;
}

int task_chain::state() const {
    return m_impl->state.load();
}

bool task_chain::finished() const {
    return (m_impl->state.load() == static_cast<int>(task_base::state::Finished));
}

bool task_chain::error() const {
    return (m_impl->state.load() == static_cast<int>(task_base::state::Failed));
}

trigger_ptr task_chain::add_task(task_base_ptr task, trigger_ptr trigger) {
    m_impl->queue.emplace(std::move(task), trigger);
    return std::move(trigger);
}

void task_chain::set_handle(const std::function<void(int)> &fnc) {
    m_impl->callback = fnc;
}

} // namespace ipc::core
