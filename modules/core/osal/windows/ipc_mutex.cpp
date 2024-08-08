#include "osal/ipc_mutex.h"
#include "osal/ipc_shared_memory.h"
#include <Windows.h>
#include <string.h>

namespace ipc::core {
#define RETURN_IF_NOT_VALID_HANDLE(handle)                  \
    if (handle == (HANDLE)NULL || handle == (HANDLE) - 1) { \
        return RET_ERR;                                     \
    }

#define GENERATE_MUTEX_NAME(from)        \
    char genName[MTX_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "mtx.%s", from);
/**
 * @fn mutex_create
 * @brief 	Create pthread mutex
 *
 * @param mtx		Mutex info structure
 * @param mtx_name	Mutex name, using in USE_SHARED_MEMORY_MUTEX type
 * @param recursive Mutex recursive
 * @return MUTEX_T
 */
int mutex_create(MUTEX_T &mtx, const char *name, unsigned int recursive) {
    HANDLE handle = NULL;
    SECURITY_DESCRIPTOR sd;
    SECURITY_ATTRIBUTES sa;

    assert(InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION));
    assert(SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    if (!name) {
        if ((handle = CreateMutexA(&sa, FALSE, name)) == NULL) {
            OSAL_ERR("[%s] Create mutex %s failed, %s\n", __FUNCTION__, name, __ERROR_STR__);
            return RET_ERR;
        }
        OSAL_INFO("[%s] Create mutex %s success\n", __FUNCTION__, name);
        mtx.lock = handle;
        return RET_OK;
    }

    GENERATE_MUTEX_NAME(name);
    strncpy(mtx.name, name, sizeof(mtx.name));

    if ((handle = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, genName)) == NULL) {
        OSAL_ERR("[%s] Open mutex %s failed, %s\n", __FUNCTION__, name, __ERROR_STR__);
        if (__ERROR__ == ERROR_FILE_NOT_FOUND) {
            if ((handle = CreateMutexA(&sa, FALSE, genName)) == NULL) {
                OSAL_ERR("[%s] Create mutex %s failed, %s\n", __FUNCTION__, name, __ERROR_STR__);
                return RET_ERR;
            }
        }
    }

    OSAL_INFO("[%s] Open mutex %s success\n", __FUNCTION__, name);
    mtx.lock = handle;
    return RET_OK;
}

/**
 * @fn mutex_lock
 * @brief Mutex lock
 *
 * @param mtx
 * @return int
 */
int mutex_lock(MUTEX_T &mtx, TIME_T timeout) {
    int ret = 0;
    RETURN_IF_NOT_VALID_HANDLE(mtx.lock);
    if (timeout > 0) {
        ret = WaitForSingleObject(mtx.lock, timeout);
    }
    else {
        ret = WaitForSingleObject(mtx.lock, INFINITE);
    }

    if (ret != WAIT_OBJECT_0) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn mutex_try_lock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
int mutex_try_lock(MUTEX_T &mtx) {
    RETURN_IF_NOT_VALID_HANDLE(mtx.lock);
    int ret = WaitForSingleObject(mtx.lock, INFINITE);
    if (ret != WAIT_OBJECT_0) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn mutex_unlock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
int mutex_unlock(MUTEX_T &mtx) {
    RETURN_IF_NOT_VALID_HANDLE(mtx.lock);
    int ret = ReleaseMutex(mtx.lock);
    if (ret == ERROR) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn mutex_destroy
 * @brief destroy mutex, mtx will be deallocated (free)
 *
 * @param mtx
 * @return int
 */
int mutex_destroy(MUTEX_T &mtx) {
    RETURN_IF_NOT_VALID_HANDLE(mtx.lock);
    int ret = CloseHandle(mtx.lock);
    if (ret == ERROR) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    mtx.lock == NULL;
    return RET_OK;
}
} // namespace ipc::core