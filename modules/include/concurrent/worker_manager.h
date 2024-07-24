#ifndef WORKER_MANAGER_H
#define WORKER_MANAGER_H

#include "concurrent/worker.h"

namespace ipc::core {
class worker_man {
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

    worker_man();
    ~worker_man();

    worker_man(const worker_man &) = delete;
    worker_man &operator=(const worker_man &) = delete;

public:
    static worker_man &get_instance();
    worker_ptr create_worker();
};
} // namespace ipc::core

#endif // WORKER_MANAGER_H