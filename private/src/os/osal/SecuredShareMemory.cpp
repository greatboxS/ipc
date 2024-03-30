#include "SecuredShareMemory.h"
#include "SharedMemory.h"
#include "Semaphore.h"
#include "Mutex.h"
#include <string.h>
#include "dbg/Debug.h"
#include "File.h"

#if defined(LINUX) || defined(__linux__)
#include <unistd.h>
#endif

namespace gbs
{
    namespace osal
    {
        SECURED_SHARE_MEMORY_T SECMEM_Create(const char *name, size_t size) {
            SECURED_SHARE_MEMORY_T secmem;
            secmem.isopen = 0;

            if (name) {
                strncpy(secmem.name, name, SHM_NAME_SIZE);
                if (SEM_Create(secmem.sem, 10, secmem.name) < 0) {
                    LOG_OSAL_ERROR("Failed to create semaphore\n");
                    return secmem;
                }

                if (MUTEX_Create(secmem.mtx, secmem.name) < 0) {
                    LOG_OSAL_ERROR("Failed to create mutex\n");
                    SEM_Destroy(secmem.sem);
                    return secmem;
                }

                if (SHM_Create(secmem.shm, secmem.name, size) < 0) {
                    LOG_OSAL_ERROR("Failed to create share-memory\n");
                    SEM_Destroy(secmem.sem);
                    MUTEX_Destroy(secmem.mtx);
                    return secmem;
                }
                secmem.isopen = 1;
            }
            return secmem;
        }

        int SECMEM_Destroy(SECURED_SHARE_MEMORY_T &secmem) {
            SEM_Destroy(secmem.sem);
            MUTEX_Destroy(secmem.mtx);
            SHM_Destroy(secmem.shm);
            secmem.isopen = 0;
            return RET_OK;
        }

        SECURED_SHARE_MEMORY_T SECMEM_Open(const char *name, size_t size) {
            SECURED_SHARE_MEMORY_T secmem;
            secmem.isopen = 0;

            if (name) {
                strncpy(secmem.name, name, SHM_NAME_SIZE);
                if (SEM_Open(secmem.sem, secmem.name) < 0) {
                    LOG_OSAL_ERROR("Failed to open semaphore\n");
                    return secmem;
                }

                if (MUTEX_Create(secmem.mtx, secmem.name) < 0) {
                    LOG_OSAL_ERROR("Failed to open mutex\n");
                    SEM_Destroy(secmem.sem);
                    return secmem;
                }

                if (SHM_Open(secmem.shm, secmem.name, size) < 0) {
                    LOG_OSAL_ERROR("Failed to create share-memory\n");
                    SEM_Destroy(secmem.sem);
                    MUTEX_Destroy(secmem.mtx);
                    return secmem;
                }
                secmem.isopen = 1;
            }
            return secmem;
        }

        int SECMEM_Close(SECURED_SHARE_MEMORY_T &secmem) {
            SEM_Close(secmem.sem);
            SHM_Close(secmem.shm);
            secmem.isopen = 0;
            return RET_OK;
        }

        int SECMEM_TakeAccess(SECURED_SHARE_MEMORY_T &secmem) {

            if (SEM_Wait(secmem.sem) != RET_OK) return RET_ERR;

            if (MUTEX_Lock(secmem.mtx) != RET_OK) {
                SEM_Post(secmem.sem);
                return RET_ERR;
            }

            return RET_OK;
        }

        int SECMEM_ReleaseAccess(SECURED_SHARE_MEMORY_T &secmem) {
            MUTEX_UnLock(secmem.mtx);
            SEM_Post(secmem.sem);
            return RET_OK;
        }

        void *SECMEM_GetBaseAddress(SECURED_SHARE_MEMORY_T &secmem) {
            return secmem.shm.virt;
        }

        int SECMEM_Read(SECURED_SHARE_MEMORY_T &secmem, char *buff, size_t size) {
            if (SECMEM_TakeAccess(secmem) < 0) return RET_ERR;
            int bytes = SHM_Read(secmem.shm, buff, size);
            SECMEM_ReleaseAccess(secmem);
            return bytes;
        }

        int SECMEM_Write(SECURED_SHARE_MEMORY_T &secmem, char *buff, size_t size) {
            if (SECMEM_TakeAccess(secmem) < 0) return RET_ERR;
            int bytes = SHM_Write(secmem.shm, buff, size);
            SECMEM_ReleaseAccess(secmem);
            return bytes;
        }

        __DLL_DECLSPEC__ int64_t SECMEM_Seek(SECURED_SHARE_MEMORY_T &secmem, int64_t pos, uint32_t type) {
#if defined(WIN32) || defined(_WIN32)
            long low = (pos & 0xFFFFFFFF);
            long high = ((pos >> 32) & 0xFFFFFFFF);
            return SetFilePointer(secmem.shm.handle, low, &high, type);
#else
            return lseek(secmem.shm.handle, pos, type);
#endif
        }

        int64_t SECMEM_CurrentPos(SECURED_SHARE_MEMORY_T &secmem) {
            return SECMEM_Seek(secmem, 0, 1);
        }
    } // namespace osal
} // namespace gbs
