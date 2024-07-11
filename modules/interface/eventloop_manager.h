#ifndef EVENTLOOP_MANAGER_H
#define EVENTLOOP_MANAGER_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include "eventloop.h"

namespace ipc::core {
class evloop_man {
    evloop_man(const evloop_man &) = delete;
    evloop_man &operator=(const evloop_man &) = delete;
    evloop_man();
    ~evloop_man();

public:
    static evloop_man &get_instance();

    std::shared_ptr<evloop> create_evloop(const std::function<void(std::shared_ptr<message>)> &handle = nullptr);
    std::weak_ptr<const evloop> get_evloop(uint64_t id);
    size_t evloop_count() const;
    void post_event(int evloop_id, std::shared_ptr<message> mesg);
    void post_event(std::weak_ptr<const evloop> evloop, std::shared_ptr<message> mesg);
    void quit();

private:
    class manager_p;
    manager_p *m_priv = nullptr;
};

} // namespace ipc::core

#endif // EVENTLOOP_MANAGER_H