/**
 * @file task.h
 * @brief Defines the `task` class template for managing and executing tasks with or without results.
 * 
 * This header file provides the definition of the `task` class template which supports task execution
 * and result management. It includes two primary templates:
 * - `task<R, Args...>`: For tasks that return a result of type `R`.
 * - `task<void, Args...>`: For tasks that do not return a result (void).
 * 
 * Each task class handles:
 * - Execution of the task function.
 * - State management (Created, Executing, Finished, Failed).
 * - Exception handling.
 * - Callback invocation upon task completion.
 * 
 * The file also includes utility functions for creating tasks:
 * - `make_task`: To create a `task` object using function objects or function pointers.
 * 
 * The `task` class template provides mechanisms to check task completion, retrieve results, and
 * handle exceptions that occur during task execution.
 */
#ifndef TASK_H
#define TASK_H

#include <exception>
#include <functional>
#include <future>
#include <atomic>
#include "task_base.h"
#include "task_helpers.h"

namespace ipc::core {

/**
 * @brief A task class template for encapsulating and executing a function with arguments.
 *
 * This class template provides a mechanism to encapsulate a function along with its arguments,
 * execute it asynchronously, and handle the result or any exception that might occur during execution.
 *
 * @tparam R The return type of the function.
 * @tparam Args The types of the arguments passed to the function.
 */
template <class R, class... Args>
class task : public task_base {
    task(const task &) = delete;
    task(task &&) = delete;
    task &operator=(const task &) = delete;
    task &operator=(task &&) = delete;

public:
    using task_fnc = std::function<R(Args...)>;                         ///< Type alias for the task function.
    using callback_fnc = std::function<void(ipc::core::task_base_ptr)>; ///< Type alias for the callback function.

    /**
     * @brief Constructs a task with the given function, callback, and arguments.
     *
     * @tparam F The type of the function object.
     * @param func The function to execute.
     * @param callback The callback to invoke after execution.
     * @param args The arguments to pass to the function.
     */
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

    /**
     * @brief Destructor.
     *
     * Cleans up any resources held by the task.
     */
    virtual ~task() = default;

    /**
     * @brief Executes the task.
     *
     * This method runs the encapsulated function with the provided arguments and updates the task's state.
     * It also invokes the callback function upon completion.
     */
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

    /**
     * @brief Returns the exception pointer if an exception was thrown during execution.
     *
     * @return An `std::exception_ptr` pointing to the exception thrown, or `nullptr` if no exception occurred.
     */
    std::exception_ptr exception_ptr() const override {
        return m_exception_ptr;
    }

    /**
     * @brief Retrieves the result of the task.
     *
     * This method blocks until the task is finished or the specified timeout occurs.
     *
     * @param ms The timeout in milliseconds.
     * @return A pointer to the `task_result` containing the result of the task, or `nullptr` if the task has no result.
     */
    const task_result *get(int ms = si_task_get_timeout) override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return m_finished; });
        return &m_task_result;
    }

    /**
     * @brief Returns the current state of the task.
     *
     * @return The state of the task as an integer.
     */
    int state() const override {
        return m_task_state.load();
    }

    /**
     * @brief Checks if the task has finished execution.
     *
     * @return `true` if the task is finished; otherwise, `false`.
     */
    bool finished() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Finished));
    }

    /**
     * @brief Checks if the task encountered an error during execution.
     *
     * @return `true` if the task has failed; otherwise, `false`.
     */
    bool error() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Failed));
    }

private:
    /**
     * @brief Handles the task execution with index sequence to unpack arguments.
     *
     * @tparam I An index sequence.
     */
    template <std::size_t... I>
    void task_handle(std::index_sequence<I...>) {
        if (m_func != nullptr) {
            m_task_result[0] = m_func(std::get<I>(m_args)...);
        }
    }

    task_fnc m_func = nullptr;                                                     ///< The function to execute.
    callback_fnc m_callback = nullptr;                                             ///< The callback function.
    std::atomic<int> m_task_state = {static_cast<int>(task_base::state::Created)}; ///< The state of the task.
    std::tuple<Args...> m_args = {};                                               ///< The arguments for the function.
    bool m_finished = false;                                                       ///< Flag indicating if the task is finished.
    task_result m_task_result = {};                                                ///< The result of the task.
    std::exception_ptr m_exception_ptr = {nullptr};                                ///< The exception pointer if an exception occurred.
    std::mutex m_mutex = {};                                                       ///< Mutex for synchronizing access.
    std::condition_variable m_condition = {};                                      ///< Condition variable for waiting.
};

/**
 * @brief Specialization of the `task` class template for tasks with `void` return type.
 *
 * This specialization handles tasks that do not return a result (i.e., the function's return type is `void`).
 *
 * @tparam Args The types of the arguments passed to the function.
 */
template <class... Args>
class task<void, Args...> : public task_base {
public:
    using task_fnc = std::function<void(Args...)>;                      ///< Type alias for the task function.
    using callback_fnc = std::function<void(ipc::core::task_base_ptr)>; ///< Type alias for the callback function.

    /**
     * @brief Constructs a task with the given function, callback, and arguments.
     *
     * @tparam F The type of the function object.
     * @param func The function to execute.
     * @param callback The callback to invoke after execution.
     * @param args The arguments to pass to the function.
     */
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

    /**
     * @brief Destructor.
     *
     * Cleans up any resources held by the task.
     */
    virtual ~task() = default;

    /**
     * @brief Executes the task.
     *
     * This method runs the encapsulated function with the provided arguments and updates the task's state.
     * It also invokes the callback function upon completion.
     */
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

    /**
     * @brief Returns the exception pointer if an exception was thrown during execution.
     *
     * @return An `std::exception_ptr` pointing to the exception thrown, or `nullptr` if no exception occurred.
     */
    std::exception_ptr exception_ptr() const override {
        return m_exception_ptr;
    }

    /**
     * @brief Retrieves the result of the task.
     *
     * This method blocks until the task is finished or the specified timeout occurs.
     *
     * @param ms The timeout in milliseconds.
     * @return Always `nullptr` for tasks with `void` return type.
     */
    const task_result *get(int ms = si_task_get_timeout) override {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_condition.wait_for(lock, std::chrono::milliseconds(ms), [this] { return m_finished; });
        return nullptr;
    }

    /**
     * @brief Returns the current state of the task.
     *
     * @return The state of the task as an integer.
     */
    int state() const override {
        return m_task_state.load();
    }

    /**
     * @brief Checks if the task has finished execution.
     *
     * @return `true` if the task is finished; otherwise, `false`.
     */
    bool finished() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Finished));
    }

    /**
     * @brief Checks if the task encountered an error during execution.
     *
     * @return `true` if the task has failed; otherwise, `false`.
     */
    bool error() const override {
        return (m_task_state.load() == static_cast<int>(task_base::state::Failed));
    }

private:
    /**
     * @brief Handles the task execution with index sequence to unpack arguments.
     *
     * @tparam I An index sequence.
     */
    template <std::size_t... I>
    void task_handle(std::index_sequence<I...>) {
        if (m_func != nullptr) {
            m_func(std::get<I>(m_args)...);
        }
    }

    task_fnc m_func = nullptr;                                                     ///< The function to execute.
    callback_fnc m_callback = nullptr;                                             ///< The callback function.
    std::atomic<int> m_task_state = {static_cast<int>(task_base::state::Created)}; ///< The state of the task.
    std::tuple<Args...> m_args = {};                                               ///< The arguments for the function.
    bool m_finished = false;                                                       ///< Flag indicating if the task is finished.
    std::exception_ptr m_exception_ptr = {nullptr};                                ///< The exception pointer if an exception occurred.
    std::mutex m_mutex = {};                                                       ///< Mutex for synchronizing access.
    std::condition_variable m_condition = {};                                      ///< Condition variable for waiting.
};

/**
 * @brief Creates a shared pointer to a task with the given function, callback, and arguments.
 *
 * @tparam F The type of the function object.
 * @tparam Args The types of the arguments passed to the function.
 * @param func The function to execute.
 * @param callback The callback to invoke after execution.
 * @param args The arguments to pass to the function.
 * @return A `std::shared_ptr` to the created task.
 */
template <typename F, typename... Args>
auto make_task(F func, std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
    using ResultType = decltype(func(std::declval<Args>()...));
    using TaskType = task<ResultType, Args...>;
    return std::make_shared<TaskType>(std::move(func), std::move(callback), std::forward<Args>(args)...);
}

/**
 * @brief Creates a shared pointer to a task with a function pointer, callback, and arguments.
 *
 * @tparam R The return type of the function.
 * @tparam Args The types of the arguments passed to the function.
 * @param func The function pointer to execute.
 * @param callback The callback to invoke after execution.
 * @param args The arguments to pass to the function.
 * @return A `std::shared_ptr` to the created task.
 */
template <typename R, typename... Args>
auto make_task(R (*func)(Args...), std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
    return std::make_shared<task<R, std::decay_t<Args>...>>(func, std::move(callback), std::forward<Args>(args)...);
}

/**
 * @brief Creates a shared pointer to a task with a bound function object, callback, and arguments.
 *
 * @tparam R The return type of the bound function object.
 * @tparam Args The types of the arguments passed to the bound function object.
 * @param func The bound function object to execute.
 * @param callback The callback to invoke after execution.
 * @param args The arguments to pass to the function object.
 * @return A `std::shared_ptr` to the created task.
 */
template <typename R, typename... Args>
auto make_task(std::_Bind<R(Args...)> func, std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
    using ReturnType = function_return_type<R>;
    return std::make_shared<task<ReturnType, std::decay_t<Args>...>>(std::move(func), std::move(callback), std::forward<Args>(args)...);
}

} // namespace ipc::core

#endif // TASK_H
