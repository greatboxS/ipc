#include "include/ipc_shm_p.h"

namespace ipc {

ipc_shm::ipc_shm(const std::string &name) {
    m_priv = new ipc_shm_private();
}

ipc_shm::~ipc_shm() {
    if (nullptr != m_priv) {
        delete m_priv;
        m_priv = nullptr;
    }
}

void ipc_shm::lock() {
    m_priv->lock();
}

void ipc_shm::unlock() {
    m_priv->lock();
}

void ipc_shm::try_lock() {
    m_priv->lock();
}

void ipc_shm::try_unlock() {
    m_priv->lock();
}

void ipc_shm::try_shared_lock() {
    m_priv->lock();
}

void ipc_shm::try_shared_unlock() {
    m_priv->lock();
}

size_t ipc_shm::size() const {
    return m_priv->size();
}

void ipc_shm::read(uint8_t *data, size_t size) {
    m_priv->size(data, size);
}

void ipc_shm::read(char *data, size_t size) {
    m_priv->read(data, size);
}

void ipc_shm::write(const uint8_t *data, size_t size) {
    m_priv->write(data, size);
}

void ipc_shm::write(const char *data, size_t size) {
    m_priv->write(data, size);
}

void ipc_shm::write(const std::string &data) {
    m_priv->write(data);
}

} // namespace ipc
