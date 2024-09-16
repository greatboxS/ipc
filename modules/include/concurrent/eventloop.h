#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include "task.h"
#include "worker.h"
#include "mesg.h"
#include "mesg_args.h"

namespace ipc::core {

class evloop_p;

class evloop {
    evloop(const evloop &) = delete;
    evloop(evloop &&) = delete;
    evloop &operator=(const evloop &) = delete;
    evloop &operator=(evloop &&) = delete;
    std::unique_ptr<evloop_p> m_impl{nullptr};

public:
    evloop(worker_ptr worker);
    virtual ~evloop();
    using handle = std::function<void(message_ptr)>;
    using handle_w_ptr = std::weak_ptr<handle>;
    using handle_s_ptr = std::shared_ptr<handle>;

    int id() const;
    bool is_running() const;
    int start();
    int stop();
    void set_handle(handle_w_ptr handle);
    const_worker_ptr get_worker() const;
    void post(message_ptr mesg);

    template <typename... Args>
    void post(const std::string &sender, const std::string &receiver, Args &&...args) {
        message_args<Args...> mesg_args;
        mesg_args.append(std::forward<Args>(args)...);
        auto mesg = message::create(sender, receiver, mesg_args.bin());
        post(std::move(mesg));
    }

    static handle_s_ptr make_handle(evloop::handle handle) {
        return std::make_shared<evloop::handle>(std::move(handle));
    }

protected:
    virtual void run(message_ptr message);
    worker_ptr worker();

};

using evloop_ptr = std::shared_ptr<evloop>;

static inline evloop_ptr make_evloop(worker_ptr worker) {
    return std::make_shared<evloop>(std::move(worker));
}

} // namespace ipc::core

#endif // EVENTLOOP_H