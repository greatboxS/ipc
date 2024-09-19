/**
 * @file condition_trigger.h
 * @brief Defines the `condition_trigger` class for managing task synchronization with timeout handling.
 *
 * This file contains the implementation of `condition_trigger`, which provides mechanisms
 * to wait for, trigger, and reset conditions with an optional timeout.
 */

#ifndef CONDITION_TRIGGER_H
#define CONDITION_TRIGGER_H

#include <memory>

namespace ipc::core {

/**
 * @class condition_trigger
 * @brief A class that handles condition waiting and triggering with timeout functionality.
 *
 * The `condition_trigger` class allows tasks to wait for a condition to be triggered,
 * optionally with a timeout. It also supports resetting the condition and checking its status.
 */
class condition_trigger {
    condition_trigger(const condition_trigger &) = delete;
    condition_trigger(condition_trigger &&) = delete;
    condition_trigger &operator=(const condition_trigger &) = delete;
    condition_trigger &operator=(condition_trigger &&) = delete;

    /**
     * @class impl
     * @brief Forward declaration of the implementation class.
     *
     * The actual implementation is hidden via the PImpl idiom.
     */
    class impl;
    impl *m_impl{nullptr};

public:
    /**
     * @brief Constructs a `condition_trigger` with an optional timeout.
     *
     * @param timeout The maximum wait time for the trigger, in milliseconds. Defaults to `INT_MAX` for no timeout.
     */
    condition_trigger(int timeout = __INT_MAX__);

    /**
     * @brief Destructor.
     *
     * Cleans up resources associated with the trigger.
     */
    ~condition_trigger();

    /**
     * @brief Blocks the calling thread until the condition is triggered.
     *
     * This method waits indefinitely for the trigger to be activated.
     */
    void wait();

    /**
     * @brief Blocks the calling thread for a specified time or until the condition is triggered.
     *
     * This method waits for a maximum of `ms` milliseconds for the trigger.
     *
     * @param ms The timeout duration in milliseconds.
     * @return `true` if the trigger was activated within the time limit, `false` if the timeout expired.
     */
    bool wait_for(int ms);

    /**
     * @brief Checks whether the trigger has been activated.
     *
     * @return `true` if the trigger is active, otherwise `false`.
     */
    bool triggered() const;

    /**
     * @brief Activates the trigger, unblocking any waiting threads.
     */
    void trigger();

    /**
     * @brief Resets the trigger to its initial state, deactivating it.
     */
    void reset();
};

/**
 * @typedef trigger_ptr
 * @brief A shared pointer to a `condition_trigger`.
 *
 * This alias makes it easier to work with shared pointers to `condition_trigger` instances.
 */
using trigger_ptr = std::shared_ptr<condition_trigger>;

/**
 * @brief Creates a new `condition_trigger` instance with the specified timeout.
 *
 * This inline function constructs a `condition_trigger` and returns it as a shared pointer.
 *
 * @param ms The timeout duration in milliseconds.
 * @return A shared pointer to the newly created `condition_trigger` instance.
 */
static inline trigger_ptr make_trigger(int ms) {
    return std::make_shared<condition_trigger>(ms);
}

} // namespace ipc::core

#endif // CONDITION_TRIGGER_H
