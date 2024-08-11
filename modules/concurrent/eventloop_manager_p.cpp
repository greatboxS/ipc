#include "eventloop_p.h"
#include "concurrent/eventloop_manager.h"
#include "worker_p.h"
#include "../identify/id_provider.h"

namespace ipc::core {

class evloop_man::manager_p {
    friend class evloop_man;

    manager_p() {}
    ~manager_p() {}

public:
    std::shared_ptr<evloop> create_evloop(evloop::handle_w_ptr handle = {}) {
        std::shared_ptr<evloop_p> loop = std::shared_ptr<evloop_p>(new evloop_p(get_new_id<id_provider_type::EventLoop>()));

        if (loop != nullptr) {
            loop->set_handle(handle);
            std::unique_lock<std::shared_mutex> lock(mtx);
            evloops[loop->id()] = loop;
        }
        return std::move(loop);
    }

    std::weak_ptr<const evloop> get_evloop(uint64_t id) {
        std::shared_ptr<evloop> loop = get_evloop_by_id(id);
        return std::weak_ptr<const evloop>(loop);
    }

    size_t evloop_count() const {
        std::shared_lock<std::shared_mutex> lock(mtx);
        return evloops.size();
    }

    void quit() {
        std::unique_lock<std::shared_mutex> lock(mtx);
        for (auto [id, ptr] : evloops) {
            if (ptr != nullptr) {
                if (ptr->is_running() == true) {
                    ptr->stop();
                }
            }
        }
    }

    void post_event(int evloop_id, message_ptr mesg) {
        auto evl = get_evloop_by_id(evloop_id);
        if (evl != nullptr) {
            evl->post(mesg);
        }
    }

    void post_event(std::shared_ptr<evloop> evloop, message_ptr mesg) {
        if (evloop != nullptr) {
            auto evl = dynamic_cast<evloop_p *>(evloop.get());
            if (evl != nullptr) {
                evl->post(mesg);
            }
        }
    }

private:
    std::shared_ptr<evloop_p> get_evloop_by_id(uint64_t id) {
        std::shared_ptr<evloop_p> loop = std::shared_ptr<evloop_p>(nullptr);
        std::shared_lock<std::shared_mutex> lock(mtx);
        auto iter = evloops.find(id);
        if (iter != evloops.end()) {
            loop = iter->second;
        }
        return std::move(loop);
    }

    mutable std::shared_mutex mtx = {};
    std::unordered_map<uint64_t, std::shared_ptr<evloop_p>> evloops = {};
    std::shared_ptr<worker> m_worker = {nullptr};
};

/**
 * @fn evloop_man()
 * @brief Construct a new evloop manager::evloop manager object
 *
 */
evloop_man::evloop_man() :
    m_priv(new manager_p()) {
}

evloop_man::~evloop_man() {
    if (m_priv != nullptr) {
        delete m_priv;
        m_priv = nullptr;
    }
}

evloop_man &evloop_man::get_instance() {
    static evloop_man instance;
    return instance;
}

std::shared_ptr<evloop> evloop_man::create_evloop(evloop::handle_w_ptr handle) {
    return m_priv->create_evloop(handle);
}

std::weak_ptr<const evloop> evloop_man::get_evloop(uint64_t id) {
    return m_priv->get_evloop(id);
}

size_t evloop_man::evloop_count() const {
    return m_priv->evloop_count();
}

void evloop_man::quit() {
    m_priv->quit();
}

void evloop_man::post_event(int evloop_id, message_ptr mesg) {
    m_priv->post_event(evloop_id, mesg);
}

void evloop_man::post_event(std::shared_ptr<evloop> evloop, message_ptr mesg) {
    m_priv->post_event(evloop, mesg);
}

} // namespace ipc::core
