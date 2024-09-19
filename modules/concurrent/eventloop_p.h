#include <shared_mutex>
#include <mutex>
#include "mesg_p.h"
#include "concurrent/eventloop.h"
#include "concurrent/worker.h"

namespace ipc::core {
class evloop_p {
    friend class evloop_man;
    friend class evloop;

public:
    enum class state {
        Created,
        Running,
        Stoped,
    };

    evloop_p(int id, worker_ptr worker);
    ~evloop_p();
    int id() const;
    bool is_running() const;
    int start();
    int stop();
    int wait();
    void set_handle(evloop::handle_w_ptr handle);
    const_worker_ptr get_worker() const;
    worker_ptr get_worker();

private:
    int get_state() const;
    void set_state(evloop_p::state s);
    void post(message_ptr mesg);
    void task_completed(ipc::core::task_base_ptr task);
    static void task_handle(message_ptr mesg, evloop::handle_w_ptr main_handle, evloop::handle_w_ptr sub_handle);

    mutable std::shared_mutex m_mtx = {};
    uint64_t m_id = 0;
    int m_state = 0;
    evloop::handle_s_ptr m_main_handle{};
    evloop::handle_w_ptr m_sub_handle{};
    std::function<void(ipc::core::task_base_ptr)> m_task_commpleted_cb = {nullptr};
    worker_ptr m_worker = nullptr;
};
} // namespace ipc::core
