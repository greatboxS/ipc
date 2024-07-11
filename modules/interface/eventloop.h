#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "ipc_def.h"
#include <functional>
#include <memory>
#include <atomic>
#include <mutex>
#include "task.h"
#include "worker.h"

namespace ipc::core {

class message;
class messagequeue;

using evloop_handle_ptr = std::weak_ptr<std::function<void(std::shared_ptr<message>)>>;

class evloop {
    evloop(const evloop &) = delete;
    evloop &operator=(const evloop &) = delete;

protected:
    evloop() = default;
    virtual ~evloop() = default;

public:
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
    virtual void set_handle(evloop_handle_ptr handle) = 0;
    virtual const worker_base *worker() const = 0;
    virtual const messagequeue *mesgqueue() const = 0;
};

} // namespace ipc::core

#endif // EVENTLOOP_H