#include <shared_mutex>
#include <mutex>
#include "eventloop.h"
#include "mesg_p.h"

namespace ipc::core {
class evloop_p : public evloop {
    friend class evloop_manager;

public:
    evloop_p(uint64_t id);
    virtual ~evloop_p();
    uint64_t id() const override;
    int status() const override;
    int start() override;
    int stop() override;
    void set_handle(const std::function<void(std::shared_ptr<message>)> &handle) override;
    const worker_base *worker() const override;
    const messagequeue *mesgqueue() const override;

private:

    std::shared_mutex m_mtx = {};
    uint64_t m_id = 0;
    int m_state = 0;
    std::function<void(std::shared_ptr<message>)> m_handle = {};
    std::shared_ptr<worker> m_worker = {nullptr};
    mesgqueue_p m_mesgqueue = {};
};
} // namespace ipc::core
