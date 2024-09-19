/**
 * @file worker.h
 * @brief Defines the base class for tasks in the IPC core framework.
 *
 * This file defines a worker class that manages and processes tasks in a separate thread, 
 * including key comments explaining its purpose, methods, and member variables.
 */

#ifndef WORKER_H
#define WORKER_H

#include "task.h"
#include "task_helpers.h"

#include <memory>
#include <vector>
#include <thread>

namespace ipc::core {

/**
 * @class worker
 * @brief The worker class is responsible for managing and executing tasks.
 */
class worker {
    worker(const worker &) = delete;
    worker(worker &&) = delete;
    worker &operator=(const worker &) = delete;
    worker &operator=(worker &&) = delete;

    class impl;
    std::unique_ptr<worker::impl> m_impl{nullptr};

public:
    /**
     * @enum State
     * @brief Enumeration representing the state of the worker.
     */
    enum State {
        Idle,      ///< Worker is idle
        Running,   ///< Worker is running
        Stopped,   ///< Worker is stopped
        Finalized, ///< Worker is finalized
        Exited,    ///< Worker has exited
    };

    /**
     * @brief Constructor that accepts a list of tasks.
     * @param task_list A vector of task_base_ptr representing the tasks.
     */
    worker(const std::vector<task_base_ptr> &task_list = {});

    /**
     * @brief Destructor.
     */
    ~worker();

    /**
     * @brief Returns the ID of the worker.
     * @return The ID of the worker.
     */
    int id() const;

    /**
     * @brief Returns the current state of the worker.
     * @return The current state of the worker.
     */
    int state() const;

    /**
     * @brief Starts the worker.
     */
    void start();

    /**
     * @brief Stops the worker.
     */
    void stop();

    /**
     * @brief Quits the worker.
     */
    void quit();

    /**
     * @brief Joins the worker thread.
     */
    void join();

    /**
     * @brief Detaches the worker thread.
     */
    void detach();

    /**
     * @brief Returns the count of executed tasks.
     * @return The count of executed tasks.
     */
    size_t executed_count() const;

    /**
     * @brief Returns the count of tasks.
     * @return The count of tasks.
     */
    size_t task_count() const;

    /**
     * @brief Assigns the worker to a specific CPU.
     * @param cpu The CPU to assign the worker to.
     */
    void assign_to(int cpu);

    /**
     * @brief Adds a task to the worker.
     * @param task A shared pointer to the task to be added.
     */
    void add_task(task_base_ptr task);

    /**
     * @brief Adds a weak task to the worker.
     * @param task A weak pointer to the task to be added.
     */
    void add_weak_task(task_base_weak_ptr task);

    /**
     * @brief Resets the worker.
     */
    void reset();

    /**
     * @brief Returns the ID of the worker thread.
     * @return The ID of the worker thread.
     */
    std::thread::id thread_id() const;

    /**
     * @brief Template method to add a task with a callback.
     * @tparam F The type of the function.
     * @tparam Args The types of the arguments.
     * @param func The function to be executed as a task.
     * @param callback The callback function to be called after task completion.
     * @param args The arguments to be passed to the function.
     * @return A shared pointer to the created task.
     */
    template <typename F, typename... Args>
    auto add_task(F func, std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
        auto new_task = make_task(std::move(func), std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    /**
     * @brief Template method to add a task with a callback (function pointer version).
     * @tparam R The return type of the function.
     * @tparam Args The types of the arguments.
     * @param func The function to be executed as a task.
     * @param callback The callback function to be called after task completion.
     * @param args The arguments to be passed to the function.
     * @return A shared pointer to the created task.
     */
    template <typename R, typename... Args>
    auto add_task(R (*func)(Args...), std::function<void(ipc::core::task_base_ptr)> callback, Args &&...args) {
        auto new_task = make_task(func, std::move(callback), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }

    /**
     * @brief Template method to add a task without a callback.
     * @tparam F The type of the function.
     * @tparam Args The types of the arguments.
     * @param func The function to be executed as a task.
     * @param args The arguments to be passed to the function.
     * @return A shared pointer to the created task.
     */
    template <typename F, typename... Args>
    auto add_nocallback_task(F func, Args &&...args) {
        auto new_task = make_task(std::move(func), std::function<void(ipc::core::task_base_ptr)>(nullptr), std::forward<Args>(args)...);
        add_task(new_task);
        return std::move(new_task);
    }
};

// Type aliases for shared pointers to worker
using worker_ptr = std::shared_ptr<worker>;
using const_worker_ptr = std::shared_ptr<const worker>;

/**
 * @brief Factory function to create a worker.
 * @param task_list A vector of task_base_ptr representing the tasks.
 * @return A shared pointer to the created worker.
 */
static inline worker_ptr make_worker(const std::vector<task_base_ptr> &task_list = {}) {
    return std::make_shared<worker>(task_list);
}

} // namespace ipc::core

#endif // WORKER_H
