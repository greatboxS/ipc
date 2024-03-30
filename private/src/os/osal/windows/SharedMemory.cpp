#include "osal/SharedMemory.h"
#include "dbg/Debug.h"
#include <stdio.h>
#include <string.h>

namespace gbs
{
    namespace osal
    {

#define GENERATE_SHM_NAME(from)          \
    char genName[SHM_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "shm.%s", from);

        /**
         * @fn SHM_Open
         * @brief Open a shared memory using POSIX shared memory
         *
         * @param shm   Memory structure that will store return data
         * @param name  Shared memory name
         * @param size  Size of shared memory to be map
         * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
         */
        int SHM_Open(SHM_T &shm, const char *name, size_t size) {
            HANDLE handle = 0;
            LPVOID virt = 0;

            if (!name) {
                _EXCEPT_THROW("SHM_Open error, invalid name!");
            }
            GENERATE_SHM_NAME(name);

            handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, genName);
            if (handle == NULL) {
                LOG_OSAL_ERROR("[%s] Open failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }

            virt = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
            if (virt == NULL || virt == (void *)-1) {
                CloseHandle(handle);
                LOG_OSAL_ERROR("[%s] Mapping shared memory address failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }

            LOG_OSAL_INFO("[%s] Open shared memory \"%s\" success\n", __FUNCTION__, name);
            strncpy(shm.name, name, sizeof(shm.name));
            shm.handle = handle;
            shm.size = size;
            shm.virt = virt;
            shm.phys = 0;
            return RET_OK;
        }

        /**
         * @fn SHM_Create
         * @brief Open a shared memory using POSIX shared memory
         *
         * @param shm   Memory structure that will store return data
         * @param name  Shared memory name
         * @param size  Size of shared memory to be map
         * @return int  0 if successed, -2 if mmap failed, otherwise is errno
         */
        int SHM_Create(SHM_T &shm, const char *name, size_t size) {
            HANDLE handle = 0;
            LPVOID virt = 0;

            SECURITY_DESCRIPTOR sd;
            SECURITY_ATTRIBUTES sa;

            assert(InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION));
            assert(SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE));
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = &sd;
            sa.bInheritHandle = FALSE;

            GENERATE_SHM_NAME(name);
            int ret = SHM_Open(shm, name, size);
            if (ret == RET_OK) return RET_OK;

            if (__ERROR__ == ERROR_FILE_NOT_FOUND) {
                if ((handle = CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, size, genName)) == NULL) {
                    LOG_OSAL_ERROR("[%s] CreateFileMappingA failed, %s\n", __FUNCTION__, __ERROR_STR__);
                    return RET_ERR;
                }
            }

            virt = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
            if (virt == NULL || virt == (void *)-1) {
                CloseHandle(handle);
                LOG_OSAL_ERROR("[%s] Mapping shared memory address failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }

            LOG_OSAL_INFO("[%s] Create new shared memory \"%s\"; addr: %p, size: %u\n", __FUNCTION__, name, virt, size);
            strncpy(shm.name, name, sizeof(shm.name));
            shm.handle = handle;
            shm.size = size;
            shm.virt = virt;
            shm.phys = 0;
            return RET_OK;
        }

        /**
         * @fn SHM_Close
         * @brief Close shared memory file descriptor
         *
         * @param shm
         * @return int
         */
        int SHM_Close(SHM_T &shm) {
            if (CloseHandle(shm.handle) == ERROR) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
        }

        /**
         * @fn SHM_Destroy
         * @brief unmap and unlink shared memory that created before
         *
         * @param shm
         * @return int
         */
        int SHM_Destroy(SHM_T &shm) {
            if (UnmapViewOfFile(shm.virt) == ERROR) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
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
            long low = (pos & 0xFFFFFFFF);
            long high = ((pos >> 32) & 0xFFFFFFFF);
            return SetFilePointer(shm.handle, low, &high, type);
        }

        /**
         * @fn SHM_CurrentPos
         * @brief
         *
         * @param shm
         * @return int64_t
         */
        __DLL_DECLSPEC__ int64_t SHM_CurrentPos(SHM_T &shm) {
            return SHM_Seek(shm, 0, FILE_CURRENT);
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
            DWORD bytes = 0;
            WriteFile(shm.handle, buff, size, &bytes, NULL);
            return bytes;
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
            DWORD bytes = 0;
            ReadFile(shm.handle, buff, size, &bytes, NULL);
            return bytes;
        }

        /**
         * @fn SHM_Sync
         * @brief
         *
         * @param shm   Shared memory file descriptor
         * @return int
         */
        int SHM_Sync(SHM_T &shm) {
            if (FlushViewOfFile(shm.virt, shm.size) == ERROR) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
        }
    } // namespace osal
} // namespace gbs