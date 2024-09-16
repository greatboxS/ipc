#include "eventloop_p.h"
#include "../identify/id_provider.h"

namespace ipc::core {

evloop::evloop(worker_ptr worker) :
    m_impl(std::make_unique<evloop_p>(get_new_id<id_provider_type::EventLoop>(), std::move(worker))) {
    m_impl->m_main_handle = make_handle(std::bind(&evloop::run, this, std::placeholders::_1));
}

evloop::~evloop() {
}

int evloop::id() const {
    return m_impl->id();
}

bool evloop::is_running() const {
    return m_impl->is_running();
}

int evloop::start() {
    return m_impl->start();
}

int evloop::stop() {
    return m_impl->stop();
}

void evloop::set_handle(handle_w_ptr handle) {
    m_impl->set_handle(handle);
}

const_worker_ptr evloop::get_worker() const {
    return m_impl->get_worker();
}

void evloop::post(message_ptr mesg) {
    m_impl->post(std::move(mesg));
}

void evloop::run(message_ptr mesg) {
    (void)(mesg);
}

std::shared_ptr<worker> evloop::worker() {
    return m_impl->get_worker();
}


} // namespace ipc::core
