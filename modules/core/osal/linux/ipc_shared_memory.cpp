#include "osal/ipc_shared_memory.h"
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

namespace ipc::core {
#ifdef SHM_NONBLOCKING_MODE
#define BLOCKING_FLAG O_NONBLOCK
#else
#define BLOCKING_FLAG 0
#endif
#define SHM_MODE (S_IRUSR | S_IWUSR)

#define GENERATE_SHM_NAME(from)          \
    char genName[SHM_NAME_SIZE + 10];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "%s_shm_osal", from);

/**
 * @fn shared_mem_open
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  size of shared memory to be map
 * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
 */
int shared_mem_open(SHM_T &shm, const char *name, size_t size) {
    int ret = 0;
    int fd = 0;
    void *virt = NULL;

    GENERATE_SHM_NAME(name);

    if ((fd = shm_open(genName, O_RDWR | BLOCKING_FLAG, SHM_MODE)) < 0) {
        OSAL_ERR("[%s] shm_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return fd;
    }

    if ((ret = ftruncate(fd, size)) < 0) {
        OSAL_ERR("[%s] ftruncate() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        close(fd);
        return ret;
    }

    if ((virt = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        OSAL_ERR("[%s] mmap() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        ret = -2;
    }

    strncpy(shm.name, name, sizeof(shm.name));
    shm.handle = fd;
    shm.size = size;
    shm.virt = virt;
    shm.phys = NULL;
    OSAL_INFO("[%s] Open shared memory: %d %p %ld\n", __FUNCTION__, shm.handle, shm.virt, shm.size);
    return ret;
}

/**
 * @fn shared_mem_create
 * @brief Create if not exit or open a existing shared memory
 * 		  using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  size of shared memory to be map
 * @return int  0 if successed, -2 if mmap failed, otherwise is errno
 */
int shared_mem_create(SHM_T &shm, const char *name, size_t size) {
    int ret = 0;
    int fd = 0;

    GENERATE_SHM_NAME(name);
    fd = shm_open(genName, O_CREAT | O_EXCL, SHM_MODE);
    if (fd < 0) {
        OSAL_ERR("[%s] shm_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return fd;
    }

    OSAL_INFO("[%s] Create new shared memory %s success\n", __FUNCTION__, name);
    close(fd);

    return shared_mem_open(shm, name, size);
}

/**
 * @fn shared_mem_close
 * @brief Close shared memory file descriptor
 *
 * @param shm
 * @return int
 */
int shared_mem_close(SHM_T &shm) {
    int ret = close(shm.handle);
    if (ret == RET_OK) {
        shm.handle = -1;
    } else {
        OSAL_INFO("[%s] Close shared memory file descriptor %d failed, %s\n", __FUNCTION__, shm.handle, __ERROR_STR__);
    }

    return ret;
}

/**
 * @fn shared_mem_destroy
 * @brief unmap and unlink shared memory that created before
 *
 * @param shm
 * @return int
 */
int shared_mem_destroy(SHM_T &shm) {
    int ret = munmap(shm.virt, shm.size);
    if (ret < 0) {
        OSAL_INFO("[%s] UnMapping shared memory virtual address failed, %s\n", __FUNCTION__, __ERROR_STR__);
    } else {
        GENERATE_SHM_NAME(shm.name);
        shm.virt = nullptr;
        shm.size = 0;
        if ((ret = shm_unlink(genName)) != 0) {
            OSAL_INFO("[%s] Unlink shared memory %s failed, %s\n", __FUNCTION__, genName, __ERROR_STR__);
        }
    }

    return ret;
}

/**
 * @fn shared_mem_seek
 * @brief
 *
 * @param shm
 * @param pos
 * @param type
 * @return int64_t
 */
int64_t shared_mem_seek(SHM_T &shm, int64_t pos, uint32_t type) {
    return lseek(shm.handle, pos, type);
}

/**
 * @fn shared_mem_current_pos
 * @brief
 *
 * @param shm
 * @return int64_t
 */
__dll_declspec__ int64_t shared_mem_current_pos(SHM_T &shm) {
    return shared_mem_seek(shm, 0, SEEK_CUR);
}

/**
 * @fn shared_mem_write
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to write
 * @param size  size to write
 * @return int  Number of bytes or -1 if error
 */
int shared_mem_write(SHM_T &shm, char *buff, size_t size) {
    return write(shm.handle, buff, size);
}

/**
 * @fn shared_mem_read
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to store the data
 * @param size  size to read
 * @return int  Number of bytes or -1 if error
 */
int shared_mem_read(SHM_T &shm, char *buff, size_t size) {
    return read(shm.handle, buff, size);
}

/**
 * @fn shared_mem_sync
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @return int
 */
int shared_mem_sync(SHM_T &shm) {
    return RET_OK;
}
} // namespace ipc::core