#include "message_queue/message_queue.h"
#include "osac/cmessage_queue.h"
#include <atomic>

namespace ipc::core {

class message_queue::impl : public cmessage_queue {
    friend class message_queue;

    std::string m_name = "";
    size_t m_msgsize = 0;
    size_t m_msgcount = 0;
    std::atomic<bool> m_created{false};
    std::atomic<bool> m_opened{false};

public:
    impl(const std::string &name, size_t msgsize, size_t msgcount) :
        m_name(name),
        m_msgsize(msgsize),
        m_msgcount(msgcount),
        m_created{false},
        m_opened{false} {
    }
    int create() {
        int ret = -1;
        if (m_created.load() == false) {
            ret = cmessage_queue::create(m_name.c_str(), m_msgsize, m_msgcount);
            if (ret == 0) {
                m_created.store(true);
            }
        }
        return ret;
    }
    int open() {
        int ret = -1;
        if (m_opened.load() == false) {
            ret = cmessage_queue::open(m_name.c_str());
            if (ret == 0) {
                m_opened.store(true);
            }
        }
        return ret;
    }

    int destroy() {
        int ret = -1;
        if (m_created.load() == true) {
            ret = cmessage_queue::destroy();
            if (ret == 0) {
                m_created.store(false);
            }
        }
        return ret;
    }

    int close() {
        int ret = -1;
        if (m_opened.load() == true) {
            ret = cmessage_queue::close();
            if (ret == 0) {
                m_opened.store(false);
            }
        }
        return ret;
    }

    bool opened() const {
        return (m_created.load() || m_opened.load());
    }
};

/**
 * @fn message_queue(const std::string &name, size_t msgsize, size_t msgcount)
 * @brief Construct a new message queue::message queue object
 *
 * @param name
 * @param msgsize
 * @param msgcount
 */
message_queue::message_queue(const std::string &name, size_t msgsize, size_t msgcount) :
    m_impl(std::make_unique<message_queue::impl>(name, msgsize, msgcount)) {
}
message_queue::~message_queue() {
}

int message_queue::create() {
    return m_impl->create();
}
int message_queue::destroy() {
    return m_impl->destroy();
}
int message_queue::open() {
    return m_impl->open();
}
int message_queue::close() {
    return m_impl->close();
}
bool message_queue::opened() const {
    return m_impl->opened();
}
int message_queue::size() {
    return m_impl->size();
}
int message_queue::send(const char *buff, size_t size) {
    return m_impl->send(buff, size);
}
int message_queue::receive(char *buff, size_t size) {
    return m_impl->receive(buff, size);
}

} // namespace ipc::core
