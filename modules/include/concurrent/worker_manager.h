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

    /**
     * @fn create_worker()
     * @brief Create a worker object
     * 
     * @param task_list task list
     * @param detach    the worker manager will not own the created task if the detach is true
     *                  otherwise a new worker will be own by worker manager.
     * @return worker_ptr 
     */
    worker_ptr create_worker(std::initializer_list<task_base_ptr> task_list = {}, bool detach = false);

    /**
     * @fn wait()
     * @brief
     *
     * @param worker    watching worker
     * @param timeout   wait time
     * @return          worker task empty status
     */
    bool wait(worker_ptr worker, int timeout = __INT_MAX__);
    void quit_all();
};
} // namespace ipc::core

#endif // WORKER_MANAGER_H