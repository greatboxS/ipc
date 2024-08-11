#include <shared_mutex>
#include <mutex>
#include "mesg_p.h"
#include "concurrent/eventloop.h"
#include "concurrent/worker.h"

namespace ipc::core {
class evloop_p : public evloop {
    friend class evloop_man;

public:
    evloop_p(uint64_t id);
    virtual ~evloop_p();
    uint64_t id() const override;
    bool is_running() const override;
    int start() override;
    int stop() override;
    void set_handle(evloop::handle_w_ptr handle) override;
    std::shared_ptr<const worker> get_worker() const override;

private:
    void post(message_ptr mesg);
    void task_completed();
    static void task_handle(message_ptr mesg, evloop::handle_w_ptr handle_ptr);

    mutable std::shared_mutex m_mtx = {};
    uint64_t m_id = 0;
    int m_state = 0;
    evloop::handle_w_ptr m_handle_ptr = {};
    worker_ptr m_worker = nullptr;
    std::function<void()> m_task_commpleted_cb = {nullptr};
};
} // namespace ipc::core
