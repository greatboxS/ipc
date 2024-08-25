#ifndef WORKER_P_H
#define WORKER_P_H

#include "concurrent/worker.h"
#
namespace ipc::core {
class worker_p : public worker {
    friend class worker_man;
    worker_p(const worker_p &) = delete;
    worker_p &operator=(const worker_p &) = delete;

    worker_p(std::initializer_list<task_base_ptr> task_list = {});

public:
    virtual ~worker_p();

    int id() const override;
    int state() const override;
    void start() override;
    void stop() override;
    void quit() override;
    void wait() override;
    void detach() override;
    size_t executed_count() const override;
    size_t task_count() const override;
    void assign_to(int cpu) override;
    void add_task(task_base_ptr task) override;
    void reset() override;

private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};
};

} // namespace ipc::core

#endif // WORKER_P_H