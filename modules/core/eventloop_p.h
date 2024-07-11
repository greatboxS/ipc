#include <shared_mutex>
#include <mutex>
#include "eventloop.h"
#include "mesg_p.h"
#include "worker.h"

namespace ipc::core {
class evloop_p : public evloop {
    friend class evloop_man;

public:
    evloop_p(uint64_t id);
    virtual ~evloop_p();
    uint64_t id() const override;
    int status() const override;
    int start() override;
    int stop() override;
    void set_handle(evloop_handle_ptr handle) override;
    const worker_base *worker() const override;
    const messagequeue *mesgqueue() const override;

private:
    void post(std::shared_ptr<message> mesg);
    void task_completed();
    static void task_handle(std::shared_ptr<message> mesg);

    std::shared_mutex m_mtx = {};
    uint64_t m_id = 0;
    int m_state = 0;
    evloop_handle_ptr m_handle_ptr = {};
    worker_ptr m_worker = nullptr;
    std::shared_ptr<mesgqueue_p> m_mesgqueue = nullptr;
    std::function<void()> m_task_commpleted_cb = {nullptr};
};
} // namespace ipc::core
