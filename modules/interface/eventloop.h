#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "common_helpers.h"
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include "task.h"
#include "worker.h"
#include "mesg.h"

namespace ipc::core {

class evloop {
    evloop(const evloop &) = delete;
    evloop &operator=(const evloop &) = delete;

protected:
    evloop() = default;
    virtual ~evloop() = default;

public:
    using handle = std::function<void(message_ptr)>;
    using handle_w_ptr = std::weak_ptr<handle>;
    using handle_s_ptr = std::shared_ptr<handle>;

    enum class state {
        Created,
        Pending,
        Running,
        Stoped,
        Invalid,
    };

    virtual uint64_t id() const = 0;
    virtual int status() const = 0;
    virtual int start() = 0;
    virtual int stop() = 0;
    virtual void set_handle(handle_w_ptr handle) = 0;
    virtual const worker_base *worker() const = 0;
    virtual const mesgqueue *queue() const = 0;

    static handle_s_ptr make_handle(evloop::handle handle) {
        return std::make_shared<evloop::handle>(std::move(handle));
    }
};

using evloop_ptr = std::shared_ptr<evloop>;

} // namespace ipc::core

#endif // EVENTLOOP_H