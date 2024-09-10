#include "worker_p.h"
#include "../identify/id_provider.h"

namespace ipc::core {

/**
 * @fn worker::worker()
 * @brief Construct a new worker::worker object
 *
 */
worker::worker(const std::vector<task_base_ptr> &task_list) :
    m_impl(std::make_unique<worker::impl>(task_list, get_new_id<id_provider_type::Worker>())) {
}

worker::~worker() {
}
int worker::id() const {
    return m_impl->id();
}

int worker::state() const {
    return m_impl->state();
}

void worker::start() {
    m_impl->start();
}

void worker::stop() {
    m_impl->stop();
}

void worker::quit() {
    m_impl->quit();
}

void worker::wait() {
    m_impl->wait();
}

void worker::detach() {
    m_impl->detach();
}

size_t worker::executed_count() const {
    return m_impl->executed_count();
}

size_t worker::task_count() const {
    return m_impl->task_count();
}

void worker::assign_to(int cpu) {
    m_impl->assign_to(cpu);
}

void worker::add_task(task_base_ptr task) {
    m_impl->add_task(task);
}

void worker::add_weak_task(task_base_weak_ptr task) {
    m_impl->add_weak_task(task);
}

void worker::reset() {
    m_impl->reset();
}

std::thread::id worker::thread_id() const {
    return m_impl->thread_id();
}
} // namespace ipc::core