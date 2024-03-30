#include "osal/SharedMemory.h"
#include "dbg/Debug.h"
#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>

namespace gbs {
namespace osal {
#ifdef SHM_NONBLOCKING_MODE
#define BLOCKING_FLAG O_NONBLOCK
#else
#define BLOCKING_FLAG 0
#endif
#define SHM_MODE (S_IRUSR | S_IWUSR)

#define GENERATE_SHM_NAME(from)          \
    char genName[SHM_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "%s_shm_osal", from);

/**
 * @fn SHM_Open
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  Size of shared memory to be map
 * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
 */
int SHM_Open(SHM_T &shm, const char *name, size_t size) {
    int ret = 0;
    int fd = 0;
    void *virt = NULL;

    GENERATE_SHM_NAME(name);

    if ((fd = shm_open(genName, O_RDWR | BLOCKING_FLAG, SHM_MODE)) < 0) {
        LOG_OSAL_ERROR("[%s] shm_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return fd;
    }

    if ((ret = ftruncate(fd, size)) < 0) {
        LOG_OSAL_ERROR("[%s] ftruncate() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        close(fd);
        return ret;
    }

    if ((virt = mmap(NULL, size, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0)) == NULL) {
        LOG_OSAL_ERROR("[%s] mmap() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        ret = -2;
    }

    strncpy(shm.name, name, sizeof(shm.name));
    shm.handle = fd;
    shm.size = size;
    shm.virt = virt;
    shm.phys = NULL;
    LOG_OSAL_INFO("[%s] Open shared memory: %d %p %ld\n", __FUNCTION__, shm.handle, shm.virt, shm.size);
    return ret;
}

/**
 * @fn SHM_Create
 * @brief Create if not exit or open a existing shared memory
 * 		  using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  Size of shared memory to be map
 * @return int  0 if successed, -2 if mmap failed, otherwise is errno
 */
int SHM_Create(SHM_T &shm, const char *name, size_t size) {
    int ret = 0;
    int fd = 0;

    ret = SHM_Open(shm, name, size);
    if (ret == 0) {
        LOG_OSAL_INFO("[%s] Create new shared memory %s success\n", __FUNCTION__, name);
        return ret;
    }

    GENERATE_SHM_NAME(name);
    fd = shm_open(genName, O_CREAT, SHM_MODE);
    if (fd < 0) {
        LOG_OSAL_ERROR("[%s] shm_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return fd;
    }

    LOG_OSAL_INFO("[%s] Create new shared memory %s success\n", __FUNCTION__, name);
    close(fd);

    return SHM_Open(shm, name, size);
}

/**
 * @fn SHM_Close
 * @brief Close shared memory file descriptor
 *
 * @param shm
 * @return int
 */
int SHM_Close(SHM_T &shm) {
    int ret = close(shm.handle);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_INFO("[%s] Close shared memory file descriptor %d failed, %s\n", __FUNCTION__, shm.handle, __ERROR_STR__);
    return ret;
}

/**
 * @fn SHM_Destroy
 * @brief unmap and unlink shared memory that created before
 *
 * @param shm
 * @return int
 */
int SHM_Destroy(SHM_T &shm) {
    int ret = munmap(shm.virt, shm.size);
    if (ret < 0) {
        LOG_OSAL_INFO("[%s] UnMapping shared memory virtual address failed, %s\n", __FUNCTION__, __ERROR_STR__);
    }
    GENERATE_SHM_NAME(shm.name);
    ret = shm_unlink(genName);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_INFO("[%s] Unlink shared memory %s failed, %s\n", __FUNCTION__, genName, __ERROR_STR__);
    return ret;
}

/**
 * @fn SHM_Seek
 * @brief
 *
 * @param shm
 * @param pos
 * @param type
 * @return int64_t
 */
int64_t SHM_Seek(SHM_T &shm, int64_t pos, uint32_t type) {
    return lseek(shm.handle, pos, type);
}

/**
 * @fn SHM_CurrentPos
 * @brief
 *
 * @param shm
 * @return int64_t
 */
__DLL_DECLSPEC__ int64_t SHM_CurrentPos(SHM_T &shm) {
    return SHM_Seek(shm, 0, SEEK_CUR);
}

/**
 * @fn SHM_Write
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to write
 * @param size  Size to write
 * @return int  Number of bytes or -1 if error
 */
int SHM_Write(SHM_T &shm, char *buff, size_t size) {
    return write(shm.handle, buff, size);
}

/**
 * @fn SHM_Read
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to store the data
 * @param size  Size to read
 * @return int  Number of bytes or -1 if error
 */
int SHM_Read(SHM_T &shm, char *buff, size_t size) {
    return read(shm.handle, buff, size);
}

/**
 * @fn SHM_Sync
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @return int
 */
int SHM_Sync(SHM_T &shm) {
    return RET_OK;
}
} // namespace osal
} // namespace gbs