#ifndef EVENTLOOP_MANAGER_H
#define EVENTLOOP_MANAGER_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include "concurrent/eventloop.h"
#include "concurrent/mesg_args.h"

namespace ipc::core {

class evloop_man {
    evloop_man(const evloop_man &) = delete;
    evloop_man &operator=(const evloop_man &) = delete;
    evloop_man();
    ~evloop_man();

public:
    static evloop_man &get_instance();

    evloop_ptr create_evloop(evloop::handle_w_ptr handle = {});
    std::weak_ptr<const evloop> get_evloop(uint64_t id);
    size_t evloop_count() const;
    void post_event(int evloop_id, message_ptr mesg);
    void post_event(evloop_ptr evloop, message_ptr mesg);

    template <typename... Args>
    void post_event(evloop_ptr evloop, const std::string &sender, const std::string &receiver, Args &&...args) {
        message_args<Args...> mesg_args;
        mesg_args.append(std::forward<Args>(args)...);
        auto mesg = message::create(sender, receiver, mesg_args.bin());
        post_event(evloop, std::move(mesg));
    }

    void quit();

private:
    class manager_p;
    manager_p *m_priv = nullptr;
};

} // namespace ipc::core

#endif // EVENTLOOP_MANAGER_H