#include "concurrent/task_chain.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>

namespace ipc::core {

class task_chain::impl {
    friend class task_chain;
    std::queue<std::pair<task_base_ptr, trigger_ptr>> _queue{};
    std::atomic<task_base::State> _state = task_base::State::Created;
    std::atomic<int> _executed = 0;
    std::exception_ptr _exception{nullptr};
    std::function<void(int)> _callback{nullptr};
    bool _finished = false;
    std::mutex _mtx{};
    task_result _result{};
    std::condition_variable _condition{};
    task_chain *_parent{nullptr};

public:
    impl(task_chain *parent) :
        _queue{},
        _state(task_base::State::Created),
        _executed(0),
        _exception{nullptr},
        _callback{nullptr},
        _finished(false),
        _mtx{},
        _result{},
        _condition{},
        _parent{parent} {
    }
    ~impl() {}

    void set_state(task_base::State s) {
        _state.store(s);
    }

    int state() const {
        return static_cast<int>(_state.load());
    }

    void execute() {
        auto &que = _queue;
        _result.clear();
        int index = 0;
        _exception = nullptr;
        try {
            set_state(task_base::State::Executing);
            if (_callback != nullptr) {
                _callback(state());
            }
            while (que.empty() == false) {
                auto p = que.front();
                que.pop();
                auto &task = p.first;
                auto &trigger = p.second;

                if (task != nullptr) {
                    /* Execute task */
                    task->execute();
                    _result[index] = task->get();
                    index += 1;

                    if (trigger == nullptr) {
                        _executed.fetch_add(1);
                        continue;
                    }

                    /* Wait for a task to be trigger for next task to be _executed*/
                    trigger->wait();
                    if (trigger->triggered() == false) {
                        set_state(task_base::State::Timeout);
                        _parent->on_task_timeout();
                        break;
                    } else {
                        _executed.fetch_add(1);
                    }
                }
            }

            /* Setting task state to Finished */
            if (state() != static_cast<int>(task_base::State::Timeout)) {
                set_state(task_base::State::Finished);
                _parent->on_task_completed();
            }
        } catch (...) {
            _exception = std::current_exception();
            set_state(task_base::State::Failed);
            _parent->on_task_failed();
        }

        {
            std::unique_lock<std::mutex> lock(_mtx);
            _finished = true;
        }
        if (_callback != nullptr) {
            _callback(state());
        }

        if (_exception != nullptr) {
            throw _exception;
        }
    }

    std::exception_ptr exception_ptr() const {
        return _exception;
    }

    const task_result *get(int ms) {
        task_result *_task_result = nullptr;
        std::unique_lock<std::mutex> lock(_mtx);
        bool done = _condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return _finished; });
        if (done == true) {
            _task_result = &_result;
        }
        return _task_result;
    }

    bool finished() const {
        return (state() == static_cast<int>(task_base::State::Finished));
    }

    bool error() const {
        return (state() == static_cast<int>(task_base::State::Failed));
    }

    trigger_ptr add_task(task_base_ptr task, trigger_ptr trigger) {
        _queue.emplace(std::move(task), trigger);
        return trigger;
    }

    void set_handle(const std::function<void(int)> &fnc) {
        _callback = fnc;
    }
};

/**
 * @fn task_chain()
 * @brief Construct a new task chain::task chain object
 *
 */
task_chain::task_chain() :
    m_impl(new task_chain::impl(this)) {
}

task_chain::~task_chain() {
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

void task_chain::execute() {
    m_impl->execute();
}

std::exception_ptr task_chain::exception_ptr() const {
    return m_impl->exception_ptr();
}

const task_result *task_chain::get(int ms) {
    return m_impl->get(ms);
}

int task_chain::state() const {
    return m_impl->state();
}

bool task_chain::finished() const {
    return m_impl->finished();
}

bool task_chain::error() const {
    return m_impl->error();
}

trigger_ptr task_chain::add_task(task_base_ptr task, trigger_ptr trigger) {
    return m_impl->add_task(std::move(task), std::move(trigger));
}

void task_chain::set_handle(const std::function<void(int)> &fnc) {
    m_impl->set_handle(fnc);
}

} // namespace ipc::core
