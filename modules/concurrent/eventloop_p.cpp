#include <unordered_map>
#include <atomic>
#include <future>
#include "eventloop_p.h"
#include "concurrent/worker_manager.h"
#include <iostream>

namespace ipc::core {

evloop_p::evloop_p(uint64_t id) :
    m_mtx{},
    m_id(id),
    m_state(static_cast<int>(state::Created)),
    m_handle_ptr({}),
    m_worker(worker_man::get_instance().create_worker()),
    m_task_commpleted_cb(std::function<void(ipc::core::task_base_ptr)>(std::bind(&evloop_p::task_completed, this, std::placeholders::_1))) {}

evloop_p::~evloop_p() {
}

uint64_t evloop_p::id() const {
    return m_id;
}

bool evloop_p::is_running() const {
    std::shared_lock<std::shared_mutex> lock(m_mtx);
    return (m_state == static_cast<int>(state::Running));
}

int evloop_p::start() {
    std::unique_lock<std::shared_mutex> lock(m_mtx);
    m_state = static_cast<int>(state::Running);
    m_worker->start();
    return 0;
}

int evloop_p::stop() {
    std::unique_lock<std::shared_mutex> lock(m_mtx);
    m_state = static_cast<int>(state::Stoped);
    m_worker->stop();
    return 0;
}

void evloop_p::set_handle(evloop::handle_w_ptr handle) {
    m_handle_ptr = handle;
}

std::shared_ptr<const worker> evloop_p::get_worker() const {
    return m_worker;
}

void evloop_p::post(message_ptr mesg) {
    m_worker->add_task(evloop_p::task_handle, m_task_commpleted_cb, std::move(mesg), evloop::handle_w_ptr(m_handle_ptr));
}

void evloop_p::task_completed(ipc::core::task_base_ptr task) {
}

void evloop_p::task_handle(message_ptr mesg, evloop::handle_w_ptr handle_ptr) {
    std::shared_ptr<std::function<void(message_ptr)>> ptr = handle_ptr.lock();
    if (ptr != nullptr) {
        (*ptr)(std::move(mesg));
    }
}

} // namespace ipc::core
