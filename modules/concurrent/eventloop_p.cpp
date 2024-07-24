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
    m_state(0),
    m_handle_ptr({}),
    m_worker(worker_man::get_instance().create_worker()),
    m_mesgqueue(std::make_shared<mesgqueue_p>(1000)),
    m_task_commpleted_cb(std::function<void()>(std::bind(&evloop_p::task_completed, this))) {}

evloop_p::~evloop_p() {
}

uint64_t evloop_p::id() const {
    return m_id;
}

int evloop_p::status() const {
    return m_worker->state();
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

const worker_base *evloop_p::worker() const {
    return nullptr;
}

const mesgqueue *evloop_p::queue() const {
    return nullptr;
}

void evloop_p::post(message_ptr mesg) {
    std::cout << "evloop_p::post\n";
    m_mesgqueue->enqueue(std::move(mesg));
    auto weak_handle = m_handle_ptr;
    m_worker->add_task(evloop_p::task_handle, m_task_commpleted_cb, std::move(m_mesgqueue->dequeue()), weak_handle);
}

void evloop_p::task_completed() {
    size_t size = m_mesgqueue->size();
    std::cout << "task_completed " << size << "\n";
    if (size > 0) {
        auto mesg = m_mesgqueue->dequeue();
        auto weak_handle = m_handle_ptr;
        m_worker->add_task(evloop_p::task_handle, m_task_commpleted_cb, std::move(mesg), weak_handle);
    }
}

void evloop_p::task_handle(message_ptr mesg, evloop::handle_w_ptr handle_ptr) {

    std::shared_ptr<std::function<void(message_ptr)>> ptr = handle_ptr.lock();
    if (ptr != nullptr) {
        (*ptr)(std::move(mesg));
    }
}

} // namespace ipc::core