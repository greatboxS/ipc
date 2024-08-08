#include "osal/ipc_shared_memory.h"
#include <stdio.h>
#include <string.h>

namespace ipc::core {

#define GENERATE_SHM_NAME(from)          \
    char genName[SHM_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "shm.%s", from);

/**
 * @fn shared_mem_open
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param shm   Memory structure that will store return data
 * @param name  Shared memory name
 * @param size  size of shared memory to be map
 * @return int  0 if successed, -1 if shm_open failed, -2 if mmap failed
 */
int shared_mem_open(SHM_T &shm, const char *name, size_t size) {
    HANDLE handle = 0;
    LPVOID virt = 0;

    if (!name) {
        _EXCEPT_THROW("shared_mem_open error, invalid name!");
    }
    GENERATE_SHM_NAME(name);

    handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, genName);
    if (handle == NULL) {
        OSAL_ERR("[%s] Open failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    virt = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (virt == NULL || virt == (void *)-1) {
        CloseHandle(handle);
        OSAL_ERR("[%s] Mapping shared memory address failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("[%s] Open shared memory \"%s\" success\n", __FUNCTION__, name);
    strncpy(shm.name, name, sizeof(shm.name));
    shm.handle = handle;
    shm.size = size;
    shm.virt = virt;
    shm.phys = 0;
    return RET_OK;
}

/**
 * @fn shared_mem_create
 * @brief Open a shared memory using POSIX shared memory
 *
 * @param shm   Memory structure that will store return data
 * @param name  Shared memory name
 * @param size  size of shared memory to be map
 * @return int  0 if successed, -2 if mmap failed, otherwise is errno
 */
int shared_mem_create(SHM_T &shm, const char *name, size_t size) {
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
    int ret = shared_mem_open(shm, name, size);
    if (ret == RET_OK) return {
        RET_OK;
    }

    if (__ERROR__ == ERROR_FILE_NOT_FOUND) {
        if ((handle = CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, size, genName)) == NULL) {
            OSAL_ERR("[%s] CreateFileMappingA failed, %s\n", __FUNCTION__, __ERROR_STR__);
            return RET_ERR;
        }
    }

    virt = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (virt == NULL || virt == (void *)-1) {
        CloseHandle(handle);
        OSAL_ERR("[%s] Mapping shared memory address failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("[%s] Create new shared memory \"%s\"; addr: %p, size: %u\n", __FUNCTION__, name, virt, size);
    strncpy(shm.name, name, sizeof(shm.name));
    shm.handle = handle;
    shm.size = size;
    shm.virt = virt;
    shm.phys = 0;
    return RET_OK;
}

/**
 * @fn shared_mem_close
 * @brief Close shared memory file descriptor
 *
 * @param shm
 * @return int
 */
int shared_mem_close(SHM_T &shm) {
    if (CloseHandle(shm.handle) == ERROR) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn shared_mem_destroy
 * @brief unmap and unlink shared memory that created before
 *
 * @param shm
 * @return int
 */
int shared_mem_destroy(SHM_T &shm) {
    if (UnmapViewOfFile(shm.virt) == ERROR) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
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
    long low = (pos & 0xFFFFFFFF);
    long high = ((pos >> 32) & 0xFFFFFFFF);
    return SetFilePointer(shm.handle, low, &high, type);
}

/**
 * @fn shared_mem_current_pos
 * @brief
 *
 * @param shm
 * @return int64_t
 */
__dll_declspec__ int64_t shared_mem_current_pos(SHM_T &shm) {
    return shared_mem_seek(shm, 0, FILE_CURRENT);
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
    DWORD bytes = 0;
    WriteFile(shm.handle, buff, size, &bytes, NULL);
    return bytes;
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
    DWORD bytes = 0;
    ReadFile(shm.handle, buff, size, &bytes, NULL);
    return bytes;
}

/**
 * @fn shared_mem_sync
 * @brief
 *
 * @param shm   Shared memory file descriptor
 * @return int
 */
int shared_mem_sync(SHM_T &shm) {
    if (FlushViewOfFile(shm.virt, shm.size) == ERROR) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}
} // namespace ipc::core