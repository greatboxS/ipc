#include "osal/ipc_semaphore.h"
#include "semaphore.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace ipc::core {
#define SEM_PSHARED 1
#ifdef SEM_NONBLOCKING_MODE
#define BLOCKING_FLAG O_NONBLOCK
#else
#define BLOCKING_FLAG 0
#endif
#define SEM_MODE (S_IRUSR | S_IWUSR)

#define GENERATE_SEM_NAME(from)                                   \
    char genName[SEM_NAME_SIZE + 12];                              \
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
 * @param value     is_initialized value
 * @return int
 */
static int SEM_Init(SEM_T &sem, int value) {
    int ret = -1;
    sem.handle = new sem_t;
    if (sem.handle != nullptr) {
        ret = sem_init(sem.handle, SEM_PSHARED, value);
        if (ret == RET_OK) {
            return RET_OK;
        }

        OSAL_ERR("[%s] sem_init() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    }
    return ret;
}

/**
 * @fn semaphore_open
 * @brief Open named sem (LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @return int
 */
int semaphore_open(SEM_T &sem, const char *name) {
    if (!name) {
        return RET_ERR;
    }
    GENERATE_SEM_NAME(name);
    sem_t *se = sem_open(genName, O_RDWR);
    if (se == SEM_FAILED) {
        OSAL_ERR("[%s] sem_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("[%s] sem_open() success\n", __FUNCTION__);
    sem.handle = se;
    strncpy(sem.name, name, sizeof(sem.name));
    return RET_OK;
}

/**
 * @fn semaphore_create
 * @brief Create (named sem LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @param value     is_initialized value
 * @return int
 */
int semaphore_create(SEM_T &sem, int value, const char *name) {
    memset(&sem, 0, sizeof(SEM_T));
    if (!name) {
        return SEM_Init(sem, value);
    }

    GENERATE_SEM_NAME(name);

    sem_t *se = sem_open(genName, O_CREAT, SEM_MODE, value);
    if (se == SEM_FAILED) {
        OSAL_ERR("[%s] sem_open() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    if (sem_close(se) < 0) {
        OSAL_ERR("[%s] sem_close() failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return semaphore_open(sem, name);
}

/**
 * @fn semaphore_wait
 * @brief Decrease sem 1 value (wait)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
int semaphore_wait(SEM_T &sem) {
    int ret = sem_wait(sem.handle);
    if (ret == RET_OK) {
        return RET_OK;
    }

    OSAL_ERR("[%s] sem_wait() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return ret;
}

/**
 * @fn semaphore_post
 * @brief Increase semaphore 1 value (release)
 *
 * @param sem
 * @return int
 */
int semaphore_post(SEM_T &sem) {
    int ret = sem_post(sem.handle);
    if (ret == RET_OK) {
        return RET_OK;
    }

    OSAL_ERR("[%s] sem_post() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return ret;
}

/**
 * @fn semaphore_value
 * @brief Get current semaphore value
 *
 * @param sem		Semaphore info structure
 * @return int      value if success, otherwise -1
 */
int semaphore_value(SEM_T &sem) {
    int value = 0;
    if (sem_getvalue(sem.handle, &value) == RET_OK) {
        return value;
    }

    OSAL_ERR("[%s] sem_getvalue() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}

/**
 * @fn semaphore_close
 * @brief Close semaphore
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
int semaphore_close(SEM_T &sem) {
    if (strlen(sem.name) == RET_OK) {
        return RET_ERR;
    }
    if (sem_close(sem.handle) == RET_OK) {
        return RET_OK;
    }

    OSAL_ERR("[%s] sem_close() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}

/**
 * @fn semaphore_destroy
 * @brief destroy semaphore
 *
 * @param sem		Semaphore info structure
 *
 * @return int  0 if success, otherwise -1
 */
int semaphore_destroy(SEM_T &sem) {
    if (strlen(sem.name) == RET_OK) {
        if (sem_destroy(sem.handle) < 0) {
            OSAL_ERR("[%s] sem_destroy() failed, %s\n", __FUNCTION__, __ERROR_STR__);
            return RET_ERR;
        }
        delete sem.handle;
        return RET_OK;
    }

    GENERATE_SEM_NAME(sem.name);
    if (sem_unlink(genName) == RET_OK) {
        return RET_OK;
    }

    OSAL_ERR("[%s] sem_unlink() failed, %s\n", __FUNCTION__, __ERROR_STR__);
    return RET_ERR;
}
} // namespace ipc::core