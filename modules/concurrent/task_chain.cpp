#include "concurrent/task_chain.h"
#include <atomic>
#include <queue>

namespace ipc::core {

class task_chain::impl {
    friend class task_chain;
    struct task_group {
        task_base_ptr task;
        trigger_ptr trigger;
    };
    std::queue<std::pair<task_base_ptr, trigger_ptr>> queue{};
    std::atomic<bool> state = static_cast<int>(task_base::state::Created);
    std::atomic<int> executed = 0;
    std::exception_ptr exception{nullptr};
    std::function<void(int)> callback{nullptr};
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

    if (m_impl->callback != nullptr) {
        m_impl->callback(m_impl->state);
    }
}

std::exception_ptr task_chain::exception_ptr() const {
    return m_impl->exception;
}

const task_result *task_chain::get(int ms) {
    return nullptr;
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
