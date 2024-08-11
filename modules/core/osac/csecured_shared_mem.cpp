#include "osac.h"
#include "csecured_shared_mem.h"
#include "osal/ipc_semaphore.h"
#include "osal/ipc_shared_memory.h"
#include "osal/ipc_mutex.h"
#include <string.h>
#include <string>
#include <random>
#include <atomic>

namespace ipc::core {

class csecured_shared_mem::impl {
    friend class csecured_shared_mem;

    std::atomic<bool> m_is_opened = false;
    std::atomic<bool> m_is_created = false;
    SEM_T m_sem = {};
    SHM_T m_shm = {};
    std::string m_name = "";
    size_t m_size = 0;

    impl(const std::string &name, size_t size) :
        m_is_opened(false),
        m_is_created(false),
        m_sem{},
        m_shm{},
        m_name(name),
        m_size(size) {
    }

    int create() {
        int ret = -1;
        do {
            if (m_is_created.load() == true) {
                OSAC_ERR("Create secured shared memory failed, already created\n");
                break;
            }

            if (m_name.length() == 0) {
                break;
            }

            if (semaphore_create(m_sem, 1, m_name.c_str()) < 0) {
                OSAC_ERR("Failed to create semaphore\n");
                break;
            }

            if (shared_mem_create(m_shm, m_name.c_str(), m_size) < 0) {
                OSAC_ERR("Failed to create share-memory\n");
                break;
            }
            m_is_created.store(true);
            ret = 0;
        } while (false);
        return ret;
    }

    int destroy() {
        int ret = -1;
        if ((ret = semaphore_destroy(m_sem)) < 0) {
            OSAC_ERR("Failed to destroy semaphore\n");
        }
        if ((ret = shared_mem_destroy(m_shm)) < 0) {
            OSAC_ERR("Failed to destroy shared mem\n");
        }
        m_is_opened.store(false);
        m_is_created.store(false);
        return ret;
    }

    int open() {
        int ret = -1;
        do {
            if (m_is_opened.load() == true) {
                OSAC_ERR("Open secured shared memory failed, already opened\n");
                break;
            }

            if (m_name.length() == 0) {
                break;
            }

            if (semaphore_open(m_sem, m_name.c_str()) < 0) {
                OSAC_ERR("Failed to open semaphore\n");
                break;
            }

            if (shared_mem_open(m_shm, m_name.c_str(), m_size) < 0) {
                OSAC_ERR("Failed to create share-memory\n");
                break;
            }
            m_is_opened.store(true);
            ret = 0;
        } while (false);
        return ret;
    }

    int opened() const {
        return (m_is_opened.load() || m_is_created.load());
    }

    void lock() {
        if (semaphore_wait(m_sem) != RET_OK) {
            return;
        }
    }

    void unlock() {
        semaphore_post(m_sem);
    }

    bool try_lock() {
        if (semaphore_wait(m_sem) != RET_OK) {
            return false;
        }

        return true;
    }

    void *get_base_addr() {
        return m_shm.virt;
    }

    size_t size() const {
        return m_shm.size;
    }

    int read(char *buff, size_t size) {
        return shared_mem_read(m_shm, buff, size);
    }

    int write(char *buff, size_t size) {
        return shared_mem_write(m_shm, buff, size);
    }

    int64_t seek(int64_t pos, uint32_t type) {
#if defined(WIN32) || defined(_WIN32)
        long low = (pos & 0xFFFFFFFF);
        long high = ((pos >> 32) & 0xFFFFFFFF);
        return SetFilePointer(secmem.m_shm.handle, low, &high, type);
#else
        return lseek(m_shm.handle, pos, type);
#endif
    }

    int64_t current_pos() {
        return seek(0, 1);
    }
};

csecured_shared_mem::csecured_shared_mem(const std::string &m_name, size_t size) :
    m_impl(new impl(m_name, size)) {
}

csecured_shared_mem::~csecured_shared_mem() {
    if (m_impl != nullptr) {
        delete m_impl;
        m_impl = nullptr;
    }
}

int csecured_shared_mem::create() {
    return m_impl->create();
}

int csecured_shared_mem::destroy() {
    return m_impl->destroy();
}

int csecured_shared_mem::open() {
    return m_impl->open();
}

int csecured_shared_mem::opened() const {
    return m_impl->opened();
}

void csecured_shared_mem::lock() {
    m_impl->lock();
}

bool csecured_shared_mem::try_lock() {
    return m_impl->try_lock();
}

void csecured_shared_mem::unlock() {
    m_impl->unlock();
}

void *csecured_shared_mem::get_base_addr() {
    return m_impl->get_base_addr();
}

size_t csecured_shared_mem::size() const {
    return m_impl->size();
}

int csecured_shared_mem::read(char *buff, size_t size) {
    return m_impl->read(buff, size);
}

int csecured_shared_mem::write(char *buff, size_t size) {
    return m_impl->write(buff, size);
}

int64_t csecured_shared_mem::seek(int64_t pos, uint32_t type) {
    return m_impl->seek(pos, type);
}

int64_t csecured_shared_mem::current_pos() {
    return m_impl->current_pos();
}

} // namespace ipc::core