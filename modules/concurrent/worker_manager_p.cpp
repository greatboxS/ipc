#include "concurrent/worker_manager.h"
#include "worker_p.h"
#include <shared_mutex>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace ipc::core {
class worker_man::impl {
    friend class worker_man;
    std::shared_mutex m_mtx{};
    std::vector<std::shared_ptr<worker>> m_worker_pool{};

public:
    impl() {}
    ~impl() {}

    worker_ptr create_worker(std::initializer_list<task_base_ptr> task_list = {}, bool detach = false) {
        auto wk = std::shared_ptr<worker>(new worker_p(task_list));
        if (detach == false) {
            m_worker_pool.push_back(wk);
        }
        return std::move(wk);
    }

    bool wait(worker_ptr worker, int timeout) {
        bool done = false;
        int time = timeout;
        using namespace std::chrono_literals;
        while (worker->task_count() > 0) {
            std::this_thread::sleep_for(1ms);
            if (--time < 0) {
                break;
            }
        }
        return (worker->task_count() == 0);
    }

    void quit_all() {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        for (auto ptr : m_worker_pool) {
            if (ptr != nullptr) {
                ptr->quit();
                ptr->wait();
            }
        }
    }
};

worker_man::worker_man() :
    m_impl(std::make_unique<worker_man::impl>()) {
}
worker_man::~worker_man() {
}

worker_man &worker_man::get_instance() {
    static worker_man instance;
    return instance;
}

worker_ptr worker_man::create_worker(std::initializer_list<task_base_ptr> task_list, bool detach) {
    return m_impl->create_worker(task_list, detach);
}

bool worker_man::wait(worker_ptr worker, int timeout) {
    return m_impl->wait(worker, timeout);
}
void ipc::core::worker_man::quit_all() {
    m_impl->quit_all();
}

} // namespace ipc::core
