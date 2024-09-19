/**
 * @file callback.h
 * @brief Defines the `callback` class for callback handling.
 *
 */

#ifndef CALLBACK_H
#define CALLBACK_H

#include <iostream>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

namespace ipc::core {

/**
 * @brief A class that manages a list of callbacks (listeners) with state tracking via weak pointers.
 *
 * This class allows registering callback functions, emitting events to all registered callbacks,
 * and managing the lifecycle of each callback via weak pointers. Callbacks are automatically removed
 * when their associated state becomes invalid.
 *
 * @tparam Args Variadic template arguments representing the parameter types that the callbacks accept.
 */
template <typename... Args>
class callback {
public:
    /// Type alias for the callback function signature.
    using call_function_t = std::function<void(Args...)>;

private:
    /// Weak pointer type used to track the state of the callback.
    using weak_state_t = std::weak_ptr<void>;
    /// Shared pointer type used for state management.
    using shared_state_t = std::shared_ptr<void>;

    /**
     * @brief Struct representing an individual callback entry.
     *
     * Each entry contains a weak pointer to the state and the actual callback function.
     */
    struct entry_t {
        weak_state_t state;       ///< Weak pointer to track the validity of the callback.
        call_function_t callback; ///< The callback function.
    };

public:
    /**
     * @brief A class that represents a connection to a registered callback.
     *
     * The connection object is returned when a callback is registered, allowing the user to manage
     * the state of the connection, such as checking its validity or resetting it.
     */
    class connect {
        friend class callback;

        /**
         * @brief Private constructor that initializes the connection with a shared state.
         *
         * @tparam T Type of the shared state object.
         * @param state A shared pointer to the state.
         */
        template <typename T>
        explicit connect(std::shared_ptr<T> state) :
            m_state(state) {}

    public:
        connect(const connect &other) :
            m_state(other.m_state) {}

        connect(connect &&other) noexcept :
            m_state(std::move(other.m_state)) {}

        connect &operator=(const connect &other) {
            if (this != &other) {
                m_state = other.m_state;
            }
            return *this;
        }

        connect &operator=(connect &&other) noexcept {
            if (this != &other) {
                m_state = std::move(other.m_state);
            }
            return *this;
        }

        ~connect() = default;

        /**
         * @brief Checks whether the connection is still valid.
         *
         * @return True if the connection is valid, false otherwise.
         */
        bool is_valid() const {
            return (m_state != nullptr);
        }

        /**
         * @brief Resets the connection, effectively invalidating it.
         */
        void reset() {
            m_state.reset();
        }

    private:
        shared_state_t m_state{nullptr}; ///< Shared state of the connection.
    };

    /**
     * @brief Registers a callback function and returns a connection object for managing the callback's state.
     *
     * @param func The callback function to register.
     * @return A connect object representing the state of the callback.
     */
    connect register_callback(call_function_t func) {
        auto state = std::make_shared<int>(1);
        m_callbacks.emplace_back(entry_t{state, std::move(func)});
        return connect(state);
    }

    /**
     * @brief Emits an event, invoking all registered callbacks with the provided arguments.
     *
     * This function will also remove any expired callbacks whose state has been invalidated.
     *
     * @param args The arguments to pass to the callbacks.
     */
    void emit(Args... args) {
        m_callbacks.erase(
            std::remove_if(m_callbacks.begin(), m_callbacks.end(), [&](entry_t &entry) {
                if (entry.state.expired()) {
                    return true;
                }
                entry.callback(std::forward<Args>(args)...);
                return false;
            }),
            m_callbacks.end());
    }

    /**
     * @brief Invokes the callbacks with the provided arguments.
     *
     * This is a shorthand for the emit function.
     *
     * @param args The arguments to pass to the callbacks.
     */
    void operator()(Args... args) {
        emit(std::forward<Args>(args)...);
    }

    /**
     * @brief Returns the number of currently registered callbacks.
     *
     * @return The number of callbacks.
     */
    auto count() const {
        return m_callbacks.size();
    }

private:
    std::vector<entry_t> m_callbacks; ///< List of registered callbacks.
};

} // namespace ipc::core

#endif // CALLBACK_H
