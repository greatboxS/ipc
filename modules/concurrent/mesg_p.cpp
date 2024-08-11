#include "mesg_p.h"
#include <string.h>
#include <atomic>
#include "../identify/id_provider.h"

namespace ipc::core {
static constexpr size_t MAXQUEUE = 1024;

/**
 * @fn message_p(const std::string &sender, const std::string &receiver, const char *data, size_t size)
 * @brief Construct a new message p::message p object
 *
 * @param sender
 * @param receiver
 * @param data
 * @param size
 */
message_p::message_p(const std::string &sender, const std::string &receiver, const char *data, size_t size) :
    m_id(get_new_id<id_provider_type::Message>()),
    m_sender(sender),
    m_receiver(receiver) {
    if ((data != nullptr) && (size > 0)) {
        m_data.resize(size);
        memcpy(m_data.data(), data, size);
    }
}

message_p::~message_p() {
}

uint64_t message_p::id() const {
    return m_id;
}

std::string message_p::sender() const {
    return m_sender;
}

std::string message_p::receiver() const {
    return m_receiver;
}

const char *message_p::data() const {
    return m_data.data();
}

size_t message_p::size() const {
    return m_data.size();
}

void message_p::setdata(const std::string &data) {
    m_data = data;
}

/**
 * @fn mesgqueue_p(size_t size)
 * @brief Construct a new mesgqueue p::mesgqueue p object
 *
 * @param size
 */
mesgqueue_p::mesgqueue_p(size_t size) :
    queuesize(size) {
}

mesgqueue_p::~mesgqueue_p() {
}

int mesgqueue_p::enqueue(message_ptr mesg) {
    int error = 0;
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (static_cast<size_t>(queue.size()) < queuesize) {
            queue.push(std::move(mesg));
        } else {
            error = -1;
        }
    }

    if (error == 0) {
        cv.notify_one();
    }
    return error;
}

message_ptr mesgqueue_p::dequeue() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return (queue.empty() == false); });
    auto msg = std::move(queue.front());
    queue.pop();
    return msg;
}

std::optional<message_ptr> mesgqueue_p::try_dequeue() {
    std::unique_lock<std::mutex> lock(mtx);
    if (queue.empty()) {
        return std::nullopt;
    }
    auto msg = std::move(queue.front());
    queue.pop();
    return msg;
}

size_t mesgqueue_p::size() const {
    std::lock_guard<std::mutex> lock(mtx);
    return queue.size();
}

message_ptr message::create(const std::string &sender, const std::string &receiver, const std::string &content) {
    return std::move(std::shared_ptr<message_p>(new message_p(sender, receiver, content.data(), content.size())));
}

} // namespace ipc::core
