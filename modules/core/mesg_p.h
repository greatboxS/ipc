#ifndef MESG_P_H
#define MESG_P_H

#include "mesg.h"
#include <queue>
#include <memory>
#include <shared_mutex>
#include <mutex>
#include <condition_variable>

namespace ipc::core {

/**
 * @fn message_p
 * @brief
 *
 */
class message_p : public message {

public:
    explicit message_p(const std::string &sender, const std::string &receiver, const char *data = nullptr, size_t size = 0);
    virtual ~message_p();
    virtual uint64_t id() const;
    virtual std::string sender() const;
    virtual std::string receiver() const;
    virtual const char *data() const;
    virtual size_t size() const;

    void setdata(const std::string &data);

private:
    uint64_t m_id = 0;
    std::string m_sender = "";
    std::string m_receiver = "";
    std::string m_data = std::string();
};

/**
 * @fn mesgqueue_p
 * @brief
 *
 */
class mesgqueue_p : public mesgqueue {
public:
    explicit mesgqueue_p(size_t size);
    virtual ~mesgqueue_p();
    int enqueue(message_ptr mesg) override;
    message_ptr dequeue() override;
    std::optional<message_ptr> try_dequeue() override;
    size_t size() const override;

private:
    std::queue<message_ptr> queue = {};
    mutable std::mutex mtx = {};
    std::condition_variable cv = {};
    size_t queuesize = 1024U;
};

} // namespace ipc::core

#endif // MESG_P_H