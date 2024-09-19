/**
 * @file task_base.h
 * @brief Defines the base class for tasks in the IPC core framework.
 *
 * This file contains the `task_base` class and its associated types and
 * constants. The `task_base` class is designed to be inherited by specific
 * task implementations and provides a base interface for executing tasks
 * and managing their states.
 */

#ifndef TASK_BASE_H
#define TASK_BASE_H

#include <memory>
#include "meta.h"

namespace ipc::core {

/**
 * @typedef task_result
 * @brief Alias for the result type used in tasks.
 *
 * The result of a task is encapsulated in a `meta_container_i` instance.
 */
using task_result = meta_container_i;

/**
 * @brief Default timeout for getting task results in milliseconds.
 *
 * The default timeout for retrieving a task's result is 20000 ms (20 seconds).
 */
static constexpr int si_task_get_timeout = 20000;

/**
 * @class task_base
 * @brief Abstract base class for all tasks in the IPC core framework.
 *
 * This class defines the interface that all tasks must implement. Tasks are
 * expected to execute asynchronously, handle exceptions, and provide their
 * results when available. The state of the task can be queried at any time.
 */
class task_base : public std::enable_shared_from_this<task_base> {
public:
    /**
     * @enum state
     * @brief Enum representing the possible states of a task.
     */
    enum class state {
        Created = 0, ///< Task has been created but not yet executed.
        Executing,   ///< Task is currently executing.
        Finished,    ///< Task has successfully completed execution.
        Failed,      ///< Task encountered an error during execution.
        Timeout,     ///< Task did not finish within the specified timeout.
    };

    virtual ~task_base() = default;

    /**
     * @brief Executes the task.
     *
     * This method should be implemented by derived classes to define the
     * specific behavior of the task during execution.
     */
    virtual void execute() = 0;

    /**
     * @brief Retrieves the exception pointer, if any, associated with the task.
     *
     * If the task encountered an exception during its execution, this method
     * returns an `std::exception_ptr` that can be used to rethrow the exception.
     *
     * @return An `std::exception_ptr` representing the exception, or `nullptr` if no exception occurred.
     */
    virtual std::exception_ptr exception_ptr() const = 0;

    /**
     * @brief Retrieves the result of the task.
     *
     * Blocks until the task completes or the specified timeout is reached.
     *
     * @param ms Timeout in milliseconds (default is `si_task_get_timeout`).
     * @return A pointer to the `task_result`, or `nullptr` if the task did not finish in time.
     */
    virtual const task_result *get(int ms = si_task_get_timeout) = 0;

    /**
     * @brief Retrieves the current state of the task.
     *
     * @return An integer representing the task's current state (as defined by the `state` enum).
     */
    virtual int state() const = 0;

    /**
     * @brief Checks if the task has finished execution.
     *
     * @return `true` if the task has finished, `false` otherwise.
     */
    virtual bool finished() const = 0;

    /**
     * @brief Checks if the task encountered an error during execution.
     *
     * @return `true` if the task encountered an error, `false` otherwise.
     */
    virtual bool error() const = 0;
};

/**
 * @typedef task_base_ptr
 * @brief Alias for a shared pointer to `task_base`.
 */
using task_base_ptr = std::shared_ptr<task_base>;

/**
 * @typedef task_base_weak_ptr
 * @brief Alias for a weak pointer to `task_base`.
 */
using task_base_weak_ptr = std::weak_ptr<task_base>;

} // namespace ipc::core

#endif // TASK_BASE_H
