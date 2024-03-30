#ifndef __SHAREDMEMORY_H__
#define __SHAREDMEMORY_H__

#include "OSAL.h"

namespace gbs {
namespace osal {
/**
 * @fn SHM_Open
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param name  Shared memory name
 * @param shm   Memory structure that will store return data
 * @param size  Size of shared memory to be map
 * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
 */
__DLL_DECLSPEC__ int SHM_Open(SHM_T &shm, const char *name, size_t size);

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
__DLL_DECLSPEC__ int SHM_Create(SHM_T &shm, const char *name, size_t size);

/**
 * @fn SHM_Close
 * @brief Close shared memory file descriptor
 *
 * @param shm
 * @return int
 */
__DLL_DECLSPEC__ int SHM_Close(SHM_T &shm);

/**
 * @fn SHM_Destroy
 * @brief unmap and unlink shared memory that created before
 *
 * @param shm
 * @return int
 */
__DLL_DECLSPEC__ int SHM_Destroy(SHM_T &shm);

/**
 * @fn SHM_Seek
 * @brief
 *
 * @param shm
 * @param pos
 * @param type
 * @return int64_t
 */
__DLL_DECLSPEC__ int64_t SHM_Seek(SHM_T &shm, int64_t pos, uint32_t type = 0);

/**
 * @fn SHM_CurrentPos
 * @brief
 *
 * @param shm
 * @return int64_t
 */
__DLL_DECLSPEC__ int64_t SHM_CurrentPos(SHM_T &shm);

/**
 * @fn SHM_Write
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to write
 * @param size  Size to write
 * @return int  Number of bytes or -1 if error
 */
__DLL_DECLSPEC__ int SHM_Write(SHM_T &shm, char *buff, size_t size);

/**
 * @fn SHM_Read
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @param buff  Buffer to store the data
 * @param size  Size to read
 * @return int  Number of bytes or -1 if error
 */
__DLL_DECLSPEC__ int SHM_Read(SHM_T &shm, char *buff, size_t size);

/**
 * @fn SHM_Sync
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @return int
 */
__DLL_DECLSPEC__ int SHM_Sync(SHM_T &shm);
}; // namespace osal
} // namespace gbs
#endif // __SHAREDMEMORY_H__