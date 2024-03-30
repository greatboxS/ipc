#include "osal/Semaphore.h"
#include "dbg/Debug.h"
#include "semaphore.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace gbs {
namespace osal {
#define SEM_PSHARED 1
#ifdef SEM_NONBLOCKING_MODE
#define BLOCKING_FLAG O_NONBLOCK
#else
#define BLOCKING_FLAG 0
#endif
#define SEM_MODE (S_IRUSR | S_IWUSR)

#define GENERATE_SEM_NAME(from)                                   \
    char genName[SEM_NAME_SIZE + 4];                              \
    memset(genName, 0, sizeof(genName));                          \
    if (from[0] == '/') {                                         \
        snprintf(genName, sizeof(genName), "%s_sem_osal", from);  \
    } else {                                                      \
        snprintf(genName, sizeof(genName), "/%s_sem_osal", from); \
    }

/**
 * @fn SEM_Init
 * @brief Initialize (unnamed sem - LINUX) or Windows
 *
 * @param sem       Semaphore pointer
 * @param value     Initialized value
 * @return int
 */
static int SEM_Init(SEM_T &sem, int value) {
    sem.handle = new sem_t;
    assert(sem.handle);
    int ret = sem_init(sem.handle, SEM_PSHARED, value);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] sem_init() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return ret;
}

/**
 * @fn SEM_Open
 * @brief Open named sem (LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @return int
 */
int SEM_Open(SEM_T &sem, const char *name) {
    if (!name) return RET_ERR;
    GENERATE_SEM_NAME(name);
    sem_t *se = sem_open(genName, O_RDWR);
    if (se == SEM_FAILED) {
        LOG_OSAL_ERROR("[%s] sem_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    LOG_OSAL_INFO("[%s] sem_open() success\n", __FUNCTION__);
    sem.handle = se;
    strncpy(sem.name, name, sizeof(sem.name));
    return RET_OK;
}

/**
 * @fn SEM_Create
 * @brief Create (named sem LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @param value     Initialized value
 * @return int
 */
int SEM_Create(SEM_T &sem, int value, const char *name) {
    memset(&sem, 0, sizeof(SEM_T));
    if (!name) {
        return SEM_Init(sem, value);
    }

    GENERATE_SEM_NAME(name);
    int ret = SEM_Open(sem, name);
    if (ret == RET_OK) return RET_OK;

    sem_t *se = sem_open(genName, O_CREAT, SEM_MODE, value);
    if (se == SEM_FAILED) {
        LOG_OSAL_ERROR("[%s] sem_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    if (sem_close(se) < 0) {
        LOG_OSAL_ERROR("[%s] sem_close() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return SEM_Open(sem, name);
}

/**
 * @fn SEM_Wait
 * @brief Decrease sem 1 value (wait)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
int SEM_Wait(SEM_T &sem) {
    int ret = sem_wait(sem.handle);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] sem_wait() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return ret;
}

/**
 * @fn SEM_Post
 * @brief Increase semaphore 1 value (release)
 *
 * @param sem
 * @return int
 */
int SEM_Post(SEM_T &sem) {
    int ret = sem_post(sem.handle);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] sem_post() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return ret;
}

/**
 * @fn SEM_Value
 * @brief Get current semaphore value
 *
 * @param sem		Semaphore info structure
 * @return int      value if success, otherwise -1
 */
int SEM_Value(SEM_T &sem) {
    int value = 0;
    if (sem_getvalue(sem.handle, &value) == RET_OK) return value;

    LOG_OSAL_ERROR("[%s] sem_getvalue() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}

/**
 * @fn SEM_Close
 * @brief Close semaphore
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
int SEM_Close(SEM_T &sem) {
    if (strlen(sem.name) == RET_OK) return RET_ERR;
    if (sem_close(sem.handle) == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] sem_close() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}

/**
 * @fn SEM_Destroy
 * @brief Destroy semaphore
 *
 * @param sem		Semaphore info structure
 *
 * @return int  0 if success, otherwise -1
 */
int SEM_Destroy(SEM_T &sem) {
    if (strlen(sem.name) == RET_OK) {
        if (sem_destroy(sem.handle) < 0) {
            LOG_OSAL_ERROR("[%s] sem_destroy() failed, %s\n", __FUNCTION__, __ERROR_STR__);
            return RET_ERR;
        }
        delete sem.handle;
        return RET_OK;
    }

    GENERATE_SEM_NAME(sem.name);
    if (sem_unlink(genName) == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] sem_unlink() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}
} // namespace osal
} // namespace gbs