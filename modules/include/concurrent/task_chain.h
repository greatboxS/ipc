/**
 * @file task_chain.h
 * @brief Defines the task_chain class for managing a chain of tasks in sequence.
 *
 * The `task_chain` class is used to execute multiple tasks in a defined order,
 * using condition triggers to control task progression.
 */

#ifndef TASK_CHAIN_H
#define TASK_CHAIN_H

#include "task_base.h"
#include "condition_trigger.h"
#include <functional>

namespace ipc::core {

/**
 * @class task_chain
 * @brief Manages a sequence of tasks, triggering the next task in line upon completion.
 *
 * The `task_chain` class inherits from `task_base` and is responsible for managing
 * and executing a sequence of tasks. Tasks can be added to the chain with associated
 * triggers, which determine when each task should execute.
 */
class task_chain : public task_base {
    task_chain(const task_chain &) = delete;
    task_chain(task_chain &&) = delete;
    task_chain &operator=(const task_chain &) = delete;
    task_chain &operator=(task_chain &&) = delete;

    class impl;
    impl *m_impl = nullptr;

public:
    /**
     * @brief Constructs a new task_chain.
     */
    task_chain();

    /**
     * @brief Destroys the task_chain and cleans up resources.
     */
    virtual ~task_chain();

    /**
     * @brief Executes the chain of tasks.
     *
     * This method triggers the execution of tasks in the chain based on their
     * associated condition triggers.
     */
    void execute() override final;

    /**
     * @brief Retrieves the exception pointer, if any, for the task chain.
     *
     * If any task in the chain encounters an exception, this method returns the
     * exception pointer for handling.
     *
     * @return An `std::exception_ptr` representing the exception, or `nullptr` if no exception occurred.
     */
    std::exception_ptr exception_ptr() const override final;

    /**
     * @brief Retrieves the result of the task chain.
     *
     * Waits for the task chain to complete or times out after a specified duration.
     *
     * @param ms Timeout in milliseconds (default is `si_task_get_timeout`).
     * @return A pointer to the `task_result`, or `nullptr` if the task chain did not finish in time.
     */
    const task_result *get(int ms = si_task_get_timeout) override final;

    /**
     * @brief Retrieves the current state of the task chain.
     *
     * @return An integer representing the current state of the task chain.
     */
    int state() const override final;

    /**
     * @brief Checks if the task chain has finished executing.
     *
     * @return `true` if the task chain has completed, `false` otherwise.
     */
    bool finished() const override final;

    /**
     * @brief Checks if the task chain encountered an error during execution.
     *
     * @return `true` if the task chain encountered an error, `false` otherwise.
     */
    bool error() const override final;

    /**
     * @brief Adds a task to the task chain with an associated condition trigger.
     *
     * The task is executed when the associated trigger is activated.
     *
     * @param task A shared pointer to the task to be added.
     * @param trigger A shared pointer to the trigger controlling the task's execution.
     * @return A shared pointer to the trigger.
     */
    trigger_ptr add_task(task_base_ptr task, trigger_ptr trigger);

    /**
     * @brief Sets a callback function to handle task chain state changes.
     *
     * The callback is invoked when the state of the task chain changes (e.g., task completed, task failed).
     *
     * @param fnc A function that accepts an integer representing the state of the task chain.
     */
    void set_handle(const std::function<void(int)> &fnc);

    /**
     * @brief Callback invoked when a task in the chain completes successfully.
     *
     * Can be overridden in derived classes to provide custom behavior.
     */
    virtual void on_task_completed() {}

    /**
     * @brief Callback invoked when a task in the chain fails.
     *
     * Can be overridden in derived classes to provide custom behavior.
     */
    virtual void on_task_failed() {}

    /**
     * @brief Callback invoked when a task in the chain times out.
     *
     * Can be overridden in derived classes to provide custom behavior.
     */
    virtual void on_task_timeout() {}

private:
};

/**
 * @typedef task_chain_ptr
 * @brief Alias for a shared pointer to `task_chain`.
 */
using task_chain_ptr = std::shared_ptr<task_chain>;

/**
 * @brief Factory function for creating a new task_chain instance.
 *
 * @return A shared pointer to the newly created `task_chain`.
 */
static inline task_chain_ptr make_task_chain() {
    return std::make_shared<task_chain>();
}

} // namespace ipc::core

#endif // TASK_CHAIN_H
