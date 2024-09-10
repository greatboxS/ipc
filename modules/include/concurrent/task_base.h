#ifndef TASK_BASE_H
#define TASK_BASE_H

#include <memory>
#include "meta.h"

namespace ipc::core
{
using task_result = meta_container_i;
static constexpr int si_task_get_timeout = 20000;

class task_base : public std::enable_shared_from_this<task_base> {
public:
    enum class state {
        Created = 0,
        Executing,
        Finished,
        Failed,
    };
    virtual ~task_base() = default;
    virtual void execute() = 0;
    virtual std::exception_ptr exception_ptr() const = 0;
    virtual const task_result *get(int ms = si_task_get_timeout) = 0;
    virtual int state() const = 0;
    virtual bool finished() const = 0;
    virtual bool error() const = 0;
};

using task_base_ptr = std::shared_ptr<task_base>;
using task_base_weak_ptr = std::weak_ptr<task_base>;
} // namespace ipc::core

#endif // TASK_BASE_H