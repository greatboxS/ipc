#ifndef TASK_H
#define TASK_H

#include <exception>
#include <functional>
#include <future>
#include <atomic>
#include "task_base.h"

namespace ipc::core {

template <class R, class... Args>
class task : public task_base {
    task(const task &) = delete;
    task &operator=(const task &) = delete;

public:
    using task_fnc = std::function<R(Args...)>;
    using callback_fnc = std::function<void(ipc::core::task_base_ptr)>;

    template <typename F>
    task(F func, callback_fnc callback, Args &&...args) :
        m_func(std::move(func)),
        m_callback(std::move(callback)),
        m_task_state(static_cast<int>(task_base::state::Created)),
        m_args(std::forward<Args>(args)...),
        m_finished(false),
        m_task_result{},
        m_exception_ptr{nullptr},
        m_mutex{},
        m_condition{} {}

    virtual ~task() = default;

    void execute() override {
        try {
            m_task_state.store(static_cast<int>(task_base::state::Executing));
            task_handle(std::index_sequence_for<Args...>{});
            m_task_state.store(static_cast<int>(task_base::state::Finished));

        } catch (...) {
            m_task_state.store(static_cast<int>(task_base::state::Failed));
            m_exception_ptr = std::current_exception();
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_finished = true;
        }
        m_condition.notify_all();
        if (m_callback != nullptr) {
            m_callback(shared_from_this());
        }
    }

    std::exception_ptr exception_ptr() const override {
        return m_exception_ptr;
    }

    const task_result *get(int ms = si_task_get_timeout) override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return m_finished; });
        return &m_task_result;
    }

    int state() const override {
        return m_task_state.load();
    }

    bool finished() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Finished));
    }

    bool error() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Failed));
    }

private:
    template <std::size_t... I>
    void task_handle(std::index_sequence<I...>) {
        if (m_func != nullptr) {
            m_task_result[0] = m_func(std::get<I>(m_args)...);
        }
    }

    task_fnc m_func = nullptr;
    callback_fnc m_callback = nullptr;
    std::atomic<int> m_task_state = {static_cast<int>(task_base::state::Created)};
    std::tuple<Args...> m_args = {};
    bool m_finished = false;
    task_result m_task_result = {};
    std::exception_ptr m_exception_ptr = {nullptr};
    std::mutex m_mutex = {};
    std::condition_variable m_condition = {};
};

/**
 * @fn task<void(Args...)> Constructor
 * @brief
 *
 * @tparam R
 * @tparam Args
 */
template <class... Args>
class task<void, Args...> : public task_base {
    task(const task &) = delete;
    task &operator=(const task &) = delete;

public:
    using task_fnc = std::function<void(Args...)>;
    using callback_fnc = std::function<void(ipc::core::task_base_ptr)>;

    template <typename F>
    task(F func, callback_fnc callback, Args &&...args) :
        m_func(std::move(func)),
        m_callback(std::move(callback)),
        m_task_state(static_cast<int>(task_base::state::Created)),
        m_args(std::forward<Args>(args)...),
        m_finished(false),
        m_exception_ptr{nullptr},
        m_mutex{},
        m_condition{} {}

    virtual ~task() = default;

    void execute() override {
        try {
            m_task_state.store(static_cast<int>(task_base::state::Executing));
            task_handle(std::index_sequence_for<Args...>{});
            m_task_state.store(static_cast<int>(task_base::state::Finished));

        } catch (...) {
            m_task_state.store(static_cast<int>(task_base::state::Failed));
            m_exception_ptr = std::current_exception();
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_finished = true;
        }
        m_condition.notify_all();
        if (m_callback != nullptr) {
            m_callback(shared_from_this());
        }
    }

    std::exception_ptr exception_ptr() const override {
        return m_exception_ptr;
    }

    const task_result *get(int ms = si_task_get_timeout) override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return m_finished; });
        return nullptr;
    }

    int state() const override {
        return m_task_state.load();
    }

    bool finished() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Finished));
    }

    bool error() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Failed));
    }

private:
    template <std::size_t... I>
    void task_handle(std::index_sequence<I...>) {
        if (m_func != nullptr) {
            m_func(std::get<I>(m_args)...);
        }
    }

    task_fnc m_func = nullptr;
    callback_fnc m_callback = nullptr;
    std::atomic<int> m_task_state = {static_cast<int>(task_base::state::Created)};
    std::tuple<Args...> m_args = {};
    bool m_finished = false;
    std::exception_ptr m_exception_ptr = {nullptr};
    std::mutex m_mutex = {};
    std::condition_variable m_condition = {};
};

template <class R, class... Args>
using task_ptr = std::shared_ptr<task<R, Args...>>;

} // namespace ipc::core

#endif // TASK_H