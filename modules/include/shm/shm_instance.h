#ifndef shm_instance_H
#define shm_instance_H

#include <stdint.h>
#include <memory>

namespace ipc::core {

class shm_instance {
private:
    class impl;
    std::unique_ptr<impl> m_impl{nullptr};

    shm_instance(const shm_instance &) = delete;
    shm_instance &operator=(const shm_instance &) = delete;

public:
    explicit shm_instance(const std::string &name, size_t size);
    ~shm_instance();

    /**
     * @fn create()
     * @brief 
     * 
     * @return true - success 
     * @return false - fail
     */
    bool create();
    void destroy();
    /**
     * @fn open()
     * @brief 
     * 
     * @return true - sucess 
     * @return false - fail
     */
    bool open();
    void close();
    bool opened() const;

    void lock();
    void unlock();
    bool try_lock();

    void *get_base_addr();
    template <typename T>
    T *get() { return reinterpret_cast<T *>(get_base_addr()); }

    size_t size() const;
    int read(char *buff, size_t size);
    int write(char *buff, size_t size);
    int64_t seek(int64_t pos, uint32_t type);
    int64_t current_pos();
};

} // namespace ipc::core

#endif // shm_instance_H