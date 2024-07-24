#include "concurrent/worker_manager.h"

namespace ipc::core {
class worker_man::impl {
    std::vector<std::weak_ptr<worker>> m_worker_pool{};

public:
    impl() {
    }
    ~impl() {
        for (auto weak_ptr_worker : m_worker_pool) {
        }
    }
    worker_ptr create_worker() {
        auto wk = std::make_shared<worker>();
        m_worker_pool.push_back(wk);
        return std::move(wk);
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

worker_ptr worker_man::create_worker() {
    return m_impl->create_worker();
}

} // namespace ipc::core
