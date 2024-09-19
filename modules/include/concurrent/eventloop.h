/**
 * @file eventloop.h
 * @brief Defines the `evloop` class for managing and executing asynchronous tasks and events.
 *
 * This header file provides the definition of the `evloop` class, which is responsible for
 * managing and executing tasks and events asynchronously. The `evloop` class handles the scheduling,
 * execution, and coordination of various tasks, ensuring efficient event-driven processing.
 *
 * Key functionalities include:
 * - **Task Scheduling**: Queue tasks for deferred execution.
 * - **Event Handling**: Manage and dispatch events to registered handlers.
 * - **Asynchronous Execution**: Support for running tasks asynchronously to prevent blocking.
 * - **Integration**: Interface with other components to ensure smooth operation within an asynchronous environment.
 *
 * The `evloop` class is essential for systems that require non-blocking operations and efficient
 * handling of concurrent tasks and events.
 */

#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>

#include "task.h"
#include "worker.h"
#include "mesg.h"
#include "mesg_args.h"

namespace ipc::core {

class evloop_p;

/**
 * @class evloop
 * @brief The evloop class is responsible for managing event loops and handling messages.
 */
class evloop {
    evloop(const evloop &) = delete;
    evloop(evloop &&) = delete;
    evloop &operator=(const evloop &) = delete;
    evloop &operator=(evloop &&) = delete;

    std::unique_ptr<evloop_p> m_impl{nullptr};

public:
    /**
     * @brief Constructor that accepts a worker.
     * @param worker A shared pointer to a worker.
     */
    evloop(worker_ptr worker);

    /**
     * @brief Destructor.
     */
    virtual ~evloop();

    /// Type alias for a message handler function
    using handle = std::function<void(message_ptr)>;

    /// Type alias for a weak pointer to a message handler function
    using handle_w_ptr = std::weak_ptr<handle>;

    /// Type alias for a shared pointer to a message handler function
    using handle_s_ptr = std::shared_ptr<handle>;

    /**
     * @brief Returns the ID of the event loop.
     * @return The ID of the event loop.
     */
    int id() const;

    /**
     * @brief Checks if the event loop is running.
     * @return True if the event loop is running, false otherwise.
     */
    bool is_running() const;

    /**
     * @brief Starts the event loop.
     * @return Status code indicating success or failure.
     */
    int start();

    /**
     * @brief Stops the event loop.
     * @return Status code indicating success or failure.
     */
    int stop();

    /**
     * @brief Waits for the event loop to finish.
     * @return Status code indicating success or failure.
     */
    int wait();

    /**
     * @brief Sets the message handler for the event loop.
     * @param handle A weak pointer to the message handler function.
     */
    void set_handle(handle_w_ptr handle);

    /**
     * @brief Returns the worker associated with the event loop.
     * @return A shared pointer to the worker.
     */
    const_worker_ptr get_worker() const;

    /**
     * @brief Posts a message to the event loop.
     * @param mesg A shared pointer to the message to be posted.
     */
    void post(message_ptr mesg);

    /**
     * @brief Template method to post a message with arguments to the event loop.
     * @tparam Args The types of the arguments.
     * @param sender The sender of the message.
     * @param receiver The receiver of the message.
     * @param args The arguments to be included in the message.
     */
    template <typename... Args>
    void post(const std::string &sender, const std::string &receiver, Args &&...args) {
        message_args<Args...> mesg_args;
        mesg_args.append(std::forward<Args>(args)...);
        auto mesg = message::create(sender, receiver, mesg_args.bin());
        post(std::move(mesg));
    }

    /**
     * @brief Factory function to create a message handler.
     * @param handle The message handler function.
     * @return A shared pointer to the created message handler.
     */
    static handle_s_ptr make_handle(evloop::handle handle) {
        return std::make_shared<evloop::handle>(std::move(handle));
    }

protected:
    /**
     * @brief Runs the event loop with the given message.
     * @param message A shared pointer to the message to be processed.
     */
    virtual void run(message_ptr message);

    /**
     * @brief Returns the worker associated with the event loop.
     * @return A shared pointer to the worker.
     */
    worker_ptr worker();
};

// Type alias for a shared pointer to evloop
using evloop_ptr = std::shared_ptr<evloop>;

/**
 * @brief Factory function to create an event loop.
 * @param worker A shared pointer to a worker.
 * @return A shared pointer to the created event loop.
 */
static inline evloop_ptr make_evloop(worker_ptr worker) {
    return std::make_shared<evloop>(std::move(worker));
}

} // namespace ipc::core

#endif // EVENTLOOP_H
