#ifndef TASK_H
#define TASK_H

#include <memory>
#include <exception>
#include <functional>
#include <future>
#include <atomic>
#include "meta.h"

namespace ipc::core {

class task_base : public std::enable_shared_from_this<task_base>{
public:
    virtual ~task_base() = default;
    virtual bool is_done() const = 0;
    virtual void execute() = 0;
    virtual meta_container_i get() = 0; 
};

using task_base_ptr = std::shared_ptr<task_base>;
using task_base_weak_ptr = std::weak_ptr<task_base>;

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
        m_done(false),
        m_args(std::forward<Args>(args)...),
        m_ready(false),
        m_result{},
        m_exception{},
        m_mutex{},
        m_condition{} {}

    virtual ~task() = default;

    void execute() override {
        try {
            task_handle(std::index_sequence_for<Args...>{});
            m_done.store(true);

        } catch (...) {
            m_exception = std::current_exception();
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ready = true;
        }
        m_condition.notify_all();
        if (m_callback != nullptr) {
            m_callback(shared_from_this());
        }
    }

    meta_container_i get() override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return m_ready; });
        if (m_exception != nullptr) {
            std::rethrow_exception(m_exception);
        }
        
        return meta_container<int>(0, m_result);
    }

    bool is_done() const override {
        return m_done.load();
    }

private:
    template <std::size_t... I>
    void task_handle(std::index_sequence<I...>) {
        if (m_func != nullptr) {
            m_result = m_func(std::get<I>(m_args)...);
        }
    }

    task_fnc m_func = nullptr;
    callback_fnc m_callback = nullptr;
    std::atomic<bool> m_done = {false};
    std::tuple<Args...> m_args = {};
    bool m_ready = false;
    R m_result = {};
    std::exception_ptr m_exception = {};
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
        m_done(false),
        m_args(std::forward<Args>(args)...),
        m_ready(false),
        m_exception{},
        m_mutex{},
        m_condition{} {}

    virtual ~task() = default;

    void execute() override {
        try {
            task_handle(std::index_sequence_for<Args...>{});
            m_done.store(true);

        } catch (...) {
            m_exception = std::current_exception();
        }

        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_ready = true;
        }
        m_condition.notify_all();
        if (m_callback != nullptr) {
            m_callback(shared_from_this());
        }
    }

    meta_container_i get() override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait(lock, [this] { return m_ready; });
        if (m_exception != nullptr) {
            std::rethrow_exception(m_exception);
        }
        return meta_container_i();
    }

    bool is_done() const override {
        return m_done.load();
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
    std::atomic<bool> m_done = {false};
    std::tuple<Args...> m_args = {};
    bool m_ready = false;
    std::exception_ptr m_exception = {};
    std::mutex m_mutex = {};
    std::condition_variable m_condition = {};
};


template <class R, class... Args>
using task_ptr = std::shared_ptr<task<R, Args...>>;

} // namespace ipc::core

#endif // TASK_H