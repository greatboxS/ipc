/**
 * @file except.h
 * @brief Defines the exception handling classes for the IPC core framework.
 *
 * This file contains the `except` and `except_base` classes, which provide
 * a custom exception handling mechanism for the IPC core framework.
 */

#ifndef EXCEPT_H
#define EXCEPT_H

#include <exception>
#include <string>
#include <string.h>
#include <stdarg.h>
#include <memory>

/**
 * @def ipc_throw_exception(...)
 * @brief Macro to throw an exception using the `except` class with file and line information.
 *
 * This macro is used to throw an exception with the current file and line number,
 * making it easier to track where the exception was raised.
 *
 * @param ... Arguments that will be forwarded to `except::raise()`.
 */
#define ipc_throw_exception(...) ipc::core::except::throw_exception(__FILE__, __LINE__, __VA_ARGS__);

namespace ipc::core {

/**
 * @class except_base
 * @brief Abstract base class for all IPC exceptions.
 *
 * This class extends `std::exception` and defines the interface for creating
 * cloneable exception types in the IPC core framework.
 */
class except_base : public std::exception {
protected:
    /**
     * @brief Virtual destructor.
     *
     * Ensures that derived exception classes are properly destroyed.
     */
    virtual ~except_base() = default;

public:
    /**
     * @brief Clones the current exception.
     *
     * This method creates a copy of the exception, allowing exceptions to be
     * propagated or rethrown with preserved information.
     *
     * @return A pointer to a new `except_base` instance, representing the cloned exception.
     */
    virtual except_base *clone() const noexcept = 0;
};

/**
 * @class except
 * @brief Custom exception class for IPC core.
 *
 * The `except` class extends `except_base` and provides additional functionality
 * for raising exceptions with formatted messages, including file and line information.
 */
class except : public except_base {
private:
    /**
     * @brief Stores the exception message.
     */
    std::string m_message = "";

protected:
    /**
     * @brief Constructor that initializes the exception with function name and line number.
     *
     * @param fnc The function name where the exception is raised.
     * @param line The line number where the exception is raised.
     */
    explicit except(const char *fnc, int line) noexcept;

    /**
     * @brief Virtual destructor.
     */
    virtual ~except() noexcept;

    /**
     * @brief Raises an exception with a formatted message.
     *
     * This method uses `printf`-style formatting to build the exception message.
     *
     * @param format The format string for the exception message.
     * @param ... Arguments for the format string.
     */
    void raise(const char *format, ...);

public:
    /**
     * @brief Clones the current exception.
     *
     * Creates a copy of the current `except` instance.
     *
     * @return A pointer to the cloned `except` instance.
     */
    except_base *clone() const noexcept override {
        return new except(*this);
    }

    /**
     * @brief Retrieves the exception message.
     *
     * @return A C-string representing the exception message.
     */
    const char *what() const noexcept override {
        return m_message.c_str();
    }

    /**
     * @brief Throws an exception with a formatted message.
     *
     * This static method throws a new `except` instance, using the provided arguments
     * to format the exception message.
     *
     * @param function The function name where the exception is raised (defaults to `__PRETTY_FUNCTION__`).
     * @param line The line number where the exception is raised (defaults to `__LINE__`).
     * @param args Arguments for formatting the exception message.
     */
    template <typename... Args>
    static void throw_exception(const char *function = __PRETTY_FUNCTION__, uint64_t line = __LINE__, Args &&...args) {
        ipc::core::except(function, line).raise(std::forward<Args>(args)...);
    }
};

} // namespace ipc::core

#endif // EXCEPT_H
