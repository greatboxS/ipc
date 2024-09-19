/**
 * @file mesg.h
 * @brief Defines the `message` class, `message_parser` template, and `mesgqueue` interface for message handling.
 *
 */

#ifndef MESG_H
#define MESG_H

#include <stdint.h>
#include <string>
#include <memory>
#include <optional>

namespace ipc::core {

/**
 * @brief Base class for messages in the IPC (Inter-Process Communication) system.
 *
 * This class provides an interface for messages that can be sent between components.
 */
class message;
using message_ptr = std::shared_ptr<message>;

/**
 * @brief Abstract base class representing a message in the IPC system.
 *
 * This class cannot be copied or assigned. Use the `create` method to instantiate a message.
 */
class message : public std::enable_shared_from_this<message> {
    message(const message &) = delete;
    message(message &&) = delete;
    message &operator=(const message &) = delete;
    message &operator=(message &&) = delete;

protected:
    /**
     * @brief Default constructor for the message class.
     */
    message() = default;

    /**
     * @brief Virtual destructor for the message class.
     */
    virtual ~message() = default;

public:
    /**
     * @brief Retrieves the unique ID of the message.
     *
     * @return A 64-bit unsigned integer representing the message ID.
     */
    virtual uint64_t id() const = 0;

    /**
     * @brief Retrieves the sender of the message.
     *
     * @return A string representing the sender's name.
     */
    virtual std::string sender() const = 0;

    /**
     * @brief Retrieves the receiver of the message.
     *
     * @return A string representing the receiver's name.
     */
    virtual std::string receiver() const = 0;

    /**
     * @brief Retrieves the content of the message.
     *
     * @return A pointer to the message data.
     */
    virtual const char *data() const = 0;

    /**
     * @brief Retrieves the size of the message content.
     *
     * @return The size of the message content in bytes.
     */
    virtual size_t size() const = 0;

    /**
     * @brief Factory method to create a new message.
     *
     * @param sender The sender of the message.
     * @param receiver The receiver of the message.
     * @param content The content of the message.
     * @return A shared pointer to the newly created message.
     */
    static message_ptr create(const std::string &sender, const std::string &receiver, const std::string &content);
};

/**
 * @brief A template class for parsing messages of a specific type.
 *
 * This class can be specialized for different message types to provide custom parsing logic.
 *
 * @tparam T The type of message to parse.
 */
template <class T>
class message_parser {
};

/**
 * @brief Abstract base class representing a message queue.
 *
 * The message queue is responsible for managing the enqueue and dequeue operations of messages.
 */
class mesgqueue {
    mesgqueue(const mesgqueue &) = delete;
    mesgqueue(mesgqueue &&) = delete;
    mesgqueue &operator=(const mesgqueue &) = delete;
    mesgqueue &operator=(mesgqueue &&) = delete;

protected:
    /**
     * @brief Default constructor for the mesgqueue class.
     */
    mesgqueue() = default;

    /**
     * @brief Virtual destructor for the mesgqueue class.
     */
    virtual ~mesgqueue() = default;

public:
    /**
     * @brief Enqueues a message into the message queue.
     *
     * @param mesg The message to enqueue.
     * @return An integer indicating the status of the enqueue operation.
     */
    virtual int enqueue(message_ptr mesg) = 0;

    /**
     * @brief Dequeues a message from the message queue.
     *
     * @return A shared pointer to the dequeued message.
     */
    virtual message_ptr dequeue() = 0;

    /**
     * @brief Attempts to dequeue a message from the message queue without blocking.
     *
     * @return An optional containing the dequeued message if available, or std::nullopt if no message is available.
     */
    virtual std::optional<message_ptr> try_dequeue() = 0;

    /**
     * @brief Retrieves the size of the message queue.
     *
     * @return The number of messages currently in the queue.
     */
    virtual size_t size() const = 0;
};

} // namespace ipc::core

#endif // MESG_H
