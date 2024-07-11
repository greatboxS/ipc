#include <unordered_map>
#include <atomic>
#include <future>
#include "eventloop_p.h"
#include "worker_manager.h"

namespace ipc::core {

static uint64_t getId() {
    static std::atomic<uint64_t> id = 0;
    id.store(id.load() + 1);
    return id.load();
}

evloop_p::evloop_p(uint64_t id) :
    m_mtx{},
    m_id(id),
    m_state(0),
    m_handle(nullptr),
    m_worker(worker_man::get_instance().create()),
    m_mesgqueue{} {
}

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

void evloop_p::set_handle(const std::function<void(std::shared_ptr<message>)> &handle) {
    m_handle = handle;
}

const worker_base *evloop_p::worker() const {
    return m_worker->get();
}

const messagequeue *evloop_p::mesgqueue() const {
    return &m_mesgqueue;
}

class evloop_manager::manager_p {
    friend class evloop_manager;

    manager_p() {
    }

    ~manager_p() {
    }

public:
    std::shared_ptr<evloop> create_evloop(const std::function<void(std::shared_ptr<message>)> &handle = nullptr) {
        std::shared_ptr<evloop> loop = std::shared_ptr<evloop>(new evloop_p(getId()));

        if (loop != nullptr) {
            if (handle == nullptr) {
                loop->set_handle(handle);
            }

            std::lock_guard<std::mutex> lock(mtx);
            evloops[loop->id()] = loop;
        }
        return std::move(loop);
    }

    std::weak_ptr<const evloop> get_evloop(uint64_t id) {
        std::shared_ptr<evloop> loop = std::shared_ptr<evloop>(nullptr);
        std::lock_guard<std::mutex> lock(mtx);
        auto iter = evloops.find(id);
        if (iter != evloops.end()) {
            loop = iter->second;
        }
        return std::weak_ptr<const evloop>(loop);
    }

    size_t evloop_count() const {
        std::lock_guard<std::mutex> lock(mtx);
        return evloops.size();
    }

    void quit() {
        std::thread *watcher_thread = new std::thread([this]() {
            while (true) {
            }
        });
    }

private:
    mutable std::mutex mtx = {};
    std::unordered_map<uint64_t, std::shared_ptr<evloop>> evloops = {};
    std::shared_ptr<worker> m_worker = {nullptr};
};
/**
 * @fn evloop_manager()
 * @brief Construct a new evloop manager::evloop manager object
 *
 */
evloop_manager::evloop_manager() :
    m_priv(new manager_p()) {
}

evloop_manager::~evloop_manager() {
    if (m_priv != nullptr) {
        delete m_priv;
        m_priv = nullptr;
    }
}

evloop_manager &evloop_manager::get_instance() {
    static evloop_manager instance;
    return instance;
}

std::shared_ptr<evloop> evloop_manager::create_evloop(const std::function<void(std::shared_ptr<message>)> &handle) {
    return m_priv->create_evloop(handle);
}

std::weak_ptr<const evloop> evloop_manager::get_evloop(uint64_t id) {
    return m_priv->get_evloop(id);
}

size_t evloop_manager::evloop_count() const {
    return m_priv->evloop_count();
}

void evloop_manager::quit() {
    m_priv->quit();
}
} // namespace ipc::core
