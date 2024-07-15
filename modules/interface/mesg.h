#ifndef MESG_H
#define MESG_H

#include <stdint.h>
#include <string>
#include <memory>
#include <optional>

namespace ipc::core {

class message;
using message_ptr = std::shared_ptr<message>;

class message : public std::enable_shared_from_this<message> {
    message(const message &) = delete;
    message &operator=(const message &) = delete;

protected:
    message() = default;
    virtual ~message() = default;

public:
    virtual uint64_t id() const = 0;
    virtual std::string sender() const = 0;
    virtual std::string receiver() const = 0;
    virtual const char *data() const = 0;
    virtual size_t size() const = 0;

    static message_ptr create(const std::string &sender, const std::string &receiver, const std::string &content);
};


template <class T>
class message_parser {
};

class mesgqueue {
    mesgqueue(const mesgqueue &) = delete;
    mesgqueue &operator=(const mesgqueue &) = delete;
protected:
    mesgqueue() = default;
    virtual ~mesgqueue() = default;
public:
    virtual int enqueue(message_ptr mesg) = 0;
    virtual message_ptr dequeue() = 0;
    virtual std::optional<message_ptr> try_dequeue() = 0;
    virtual size_t size() const = 0;
};
} // namespace ipc::core

#endif // MESG_H