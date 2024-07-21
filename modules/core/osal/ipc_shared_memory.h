#ifndef IPC_SHARED_MEMORY_H
#define IPC_SHARED_MEMORY_H

#include "osal.h"

namespace ipc::core {
/**
 * @fn shared_mem_open
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  size of shared memory to be map
 * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
 */
__dll_declspec__ int shared_mem_open(SHM_T &shm, const char *name, size_t size);

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
__dll_declspec__ int shared_mem_create(SHM_T &shm, const char *name, size_t size);

/**
 * @fn shared_mem_close
 * @brief Close shared memory file descriptor
 *
 * @param shm
 * @return int
 */
__dll_declspec__ int shared_mem_close(SHM_T &shm);

/**
 * @fn shared_mem_destroy
 * @brief unmap and unlink shared memory that created before
 *
 * @param shm
 * @return int
 */
__dll_declspec__ int shared_mem_destroy(SHM_T &shm);

/**
 * @fn shared_mem_seek
 * @brief
 *
 * @param shm
 * @param pos
 * @param type
 * @return int64_t
 */
__dll_declspec__ int64_t shared_mem_seek(SHM_T &shm, int64_t pos, uint32_t type = 0);

/**
 * @fn shared_mem_current_pos
 * @brief
 *
 * @param shm
 * @return int64_t
 */
__dll_declspec__ int64_t shared_mem_current_pos(SHM_T &shm);

/**
 * @fn shared_mem_write
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to write
 * @param size  size to write
 * @return int  Number of bytes or -1 if error
 */
__dll_declspec__ int shared_mem_write(SHM_T &shm, char *buff, size_t size);

/**
 * @fn shared_mem_read
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to store the data
 * @param size  size to read
 * @return int  Number of bytes or -1 if error
 */
__dll_declspec__ int shared_mem_read(SHM_T &shm, char *buff, size_t size);

/**
 * @fn shared_mem_sync
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @return int
 */
__dll_declspec__ int shared_mem_sync(SHM_T &shm);
} // namespace ipc::core
#endif // IPC_SHARED_MEMORY_H