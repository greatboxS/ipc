#include "shm/shm_instance.h"
#include "osac/csecured_shared_mem.h"

namespace ipc::core {

class shm_instance::impl : public csecured_shared_mem {

    friend class shm_instance;

public:
    impl(const std::string &name, size_t size) :
        csecured_shared_mem(name, size) {
    }
    ~impl() = default;
};

/**
 * @fn shm_instance(const std::string &name, size_t size)
 * @brief Construct a new shm instace::shm instace object
 *
 * @param name
 * @param size
 */
shm_instance::shm_instance(const std::string &name, size_t size) :
    m_impl(std::make_unique<shm_instance::impl>(name, size)) {
}
shm_instance::~shm_instance() {
}

int shm_instance::create() {
    return m_impl->create();
}
int shm_instance::destroy() {
    return m_impl->destroy();
}
int shm_instance::open() {
    return m_impl->open();
}
int shm_instance::close() {
    return m_impl->close();
}
int shm_instance::opened() const {
    return m_impl->opened();
}

void shm_instance::lock() {
    return m_impl->lock();
}
void shm_instance::unlock() {
    return m_impl->unlock();
}
bool shm_instance::try_lock() {
    return m_impl->try_lock();
}

void *shm_instance::get_base_addr() {
    return m_impl->get_base_addr();
}

size_t shm_instance::size() const {
    return m_impl->size();
}
int shm_instance::read(char *buff, size_t size) {
    return m_impl->read(buff, size);
}
int shm_instance::write(char *buff, size_t size) {
    return m_impl->write(buff, size);
}
int64_t shm_instance::seek(int64_t pos, uint32_t type) {
    return m_impl->seek(pos, type);
}
int64_t shm_instance::current_pos() {
    return m_impl->current_pos();
}
} // namespace ipc::core
