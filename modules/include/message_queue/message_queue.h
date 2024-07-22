#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <memory>
#include <stdint.h>
#include <string>

namespace ipc::core {
class message_queue {
private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};
    message_queue(const message_queue &) = delete;
    message_queue &operator=(const message_queue &) = delete;

public:
    message_queue(const std::string &name, size_t msgsize, size_t msgcount);
    ~message_queue();

    int create();
    int destroy();
    int open();
    int close();
    bool opened() const;
    int size();
    int send(const char *buff, size_t size);
    int receive(char *buff, size_t size);
};
} // namespace ipc::core

#endif // MESSAGE_QUEUE_H