#ifndef TASK_CHAIN_H
#define TASK_CHAIN_H

#include "task_base.h"
#include "condition_trigger.h"
#include <functional>

namespace ipc::core {

class task_chain : public task_base {
    task_chain(const task_chain &) = delete;
    task_chain(task_chain &&) = delete;
    task_chain &operator=(const task_chain &) = delete;
    class impl;
    impl *m_impl = nullptr;

public:
    task_chain();
    virtual ~task_chain();

    void execute() override final;
    std::exception_ptr exception_ptr() const override final;
    const task_result *get(int ms = si_task_get_timeout) override final;
    int state() const override final;
    bool finished() const override final;
    bool error() const override final;
    trigger_ptr add_task(task_base_ptr task, trigger_ptr trigger);
    void set_handle(const std::function<void(int)> &fnc);

    virtual void on_task_completed() {}
    virtual void on_task_failed() {}
    virtual void on_task_timeout() {}

private:
};

using task_chain_ptr = std::shared_ptr<task_chain>;
static inline task_chain_ptr make_task_chain() {
    return std::make_shared<task_chain>();
}

} // namespace ipc::core

#endif // TASK_CHAIN_H