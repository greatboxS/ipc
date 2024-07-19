#ifndef WORKER_MANAGER_H
#define WORKER_MANAGER_H

#include "concurrent/worker.h"

namespace ipc::core {
class worker_man {
    worker_man() = default;
    ~worker_man() = default;

    worker_man(const worker_man &) = delete;
    worker_man &operator=(const worker_man &) = delete;

public:
    static worker_man &get_instance() {
        static worker_man sworker_manager;
        return sworker_manager;
    }

    worker_ptr create() {
        auto wk = std::make_shared<worker>();
        m_workers.push_back(wk);
        return std::move(wk);
    }

private:
    std::vector<worker_ptr> m_workers = {};
};
} // namespace ipc::core

#endif // WORKER_MANAGER_H