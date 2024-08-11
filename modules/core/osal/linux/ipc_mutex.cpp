#include "osal/ipc_mutex.h"
#include "osal/ipc_shared_memory.h"
#include <string.h>
#include <cstdio>
#include <sys/types.h>

namespace ipc::core {

#define GENERATE_MUTEX_NAME(from)        \
    char genName[MTX_NAME_SIZE - 5];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "%s_mtx", from);

/**
 * @fn mutex_create
 * @brief 	Create pthread mutex
 *
 * @param mtx		Mutex info structure
 * @param name	Mutex name, using in USE_SHARED_MEMORY_MUTEX type
 * @param recursive Mutex recursive
 * @return int
 */
int mutex_create(MUTEX_T &mtx, const char *name, unsigned int recursive) {
    pthread_mutexattr_t attr;
    int ret = 0;
    bool multiprocess = false;
    /* Mutex recusive, which can be unlock when no lock is called*/
    int type = static_cast<int>(((recursive > 0) ? (PTHREAD_MUTEX_RECURSIVE) : PTHREAD_MUTEX_ERRORCHECK));

    memset(&mtx, 0, sizeof(MUTEX_t));

    if ((name != nullptr) && (strnlen(name, MTX_NAME_SIZE)) > 0) {
        GENERATE_MUTEX_NAME(name);

        if (shared_mem_create(mtx.mem, genName, sizeof(pthread_mutex_t)) < 0) {
            if (shared_mem_open(mtx.mem, genName, sizeof(pthread_mutex_t)) < 0) {
                OSAL_ERR("Create pthread_mutex_t with shared memory failed\n");
                return RET_ERR;
            }
        }
        strncpy(mtx.name, name, sizeof(mtx.name));
        snprintf(mtx.mem.name, sizeof(mtx.mem.name), "%s", genName);
        // Use shared memory to allocate MTX
        mtx.lock = (pthread_mutex_t *)mtx.mem.virt;
        multiprocess = true;
    } else {
        mtx.lock = new pthread_mutex_t;
        if (mtx.lock == nullptr) {
            goto exit_error;
        }
    }
    if (pthread_mutexattr_init(&attr) != RET_OK) {
        OSAL_ERR("pthread_mutex_attr_init() error (%s)", __ERROR_STR__);
        goto exit_error;
    }

    /**
     * The mutex can be shared with other thread that has the access
     * to this mutex, even if the mutex is allocated in memory that is
     * shared by multiple processes
     */
    if ((ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != RET_OK) {
        OSAL_ERR("pthread_mutexattr_setpshared() error %s\n", __ERROR_STR__);
    }
    if ((ret = pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST)) != RET_OK) {
        OSAL_ERR("pthread_mutexattr_setpshared() error %s\n", __ERROR_STR__);
    }

    if ((ret = pthread_mutexattr_settype(&attr, type)) != RET_OK) {
        OSAL_ERR("pthread_mutexattr_settype() error %s\n", __ERROR_STR__);
    }
    /* Mutex attr protocol set */
    if ((ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT)) != RET_OK) {
        OSAL_ERR("pthread_mutexattr_setprotocol() error %s\n", __ERROR_STR__);
    }
    /* Mutex init */
    if ((ret = pthread_mutex_init(mtx.lock, &attr)) != RET_OK) {
        OSAL_ERR("pthread_mutex_init() error %s\n", __ERROR_STR__);
        pthread_mutexattr_destroy(&attr);
        goto exit_error;
    }
    /* destroy initialized mutex attribute */
    if ((ret = pthread_mutexattr_destroy(&attr)) != RET_OK) {
        OSAL_ERR("pthread_mutexattr_destroy() error %s\n", __ERROR_STR__);
    }
    OSAL_INFO("[%s] Create mutex %s success\n", __FUNCTION__, mtx.mem.name);
    return RET_OK;

exit_error:
    if (multiprocess == true) {
        shared_mem_destroy(mtx.mem);
        shared_mem_close(mtx.mem);
    } else {
        delete mtx.lock;
    }
    mtx.lock = nullptr;
    return RET_ERR;
}

/**
 * @fn mutex_lock
 * @brief Mutex lock
 *
 * @param mtx
 * @param timeout 	Wait for mutex available after timeout (ms)
 * @return int
 */
int mutex_lock(MUTEX_T &mtx, long int timeout) {
    int ret = 0;

    if (mtx.lock == nullptr) {
        return RET_ERR;
    }

    if (timeout > 0) {
        timespec time;
        long int sec = (int)(timeout / 1000.0f);
        long int nano = (timeout - sec * 1000) * 1000;
        time.tv_sec = sec;
        time.tv_nsec = nano;
        if ((ret = pthread_mutex_timedlock(mtx.lock, &time)) != RET_OK) {
            OSAL_ERR("[%s] Mutex timelock failed, %d\n", __FUNCTION__, ret);
            return RET_ERR;
        }
    } else {
        ret = pthread_mutex_lock(mtx.lock);
    }

    if (ret == EOWNERDEAD) {
        OSAL_ERR("Mutex owner died, making it consistent");
        pthread_mutex_consistent(mtx.lock);
        return ret;
    }

    if (ret == RET_OK) {
        return RET_OK;
    }

    OSAL_ERR("[%s] Mutex lock failed, %d\n", __FUNCTION__, ret);
    return RET_ERR;
}

/**
 * @fn mutex_try_lock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
int mutex_try_lock(MUTEX_T &mtx) {
    if (mtx.lock == nullptr) {
        return RET_ERR;
    }
    int ret = pthread_mutex_trylock(mtx.lock);
    if (ret == EOWNERDEAD) {
        OSAL_ERR("Mutex owner died, making it consistent");
        pthread_mutex_consistent(mtx.lock);
    }

    if (ret == RET_OK) {
        return ret;
    }

    OSAL_ERR("[%s] Mutex trylock failed, %d\n", __FUNCTION__, ret);
    return RET_ERR;
}

/**
 * @fn mutex_unlock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
int mutex_unlock(MUTEX_T &mtx) {
    if (mtx.lock == nullptr) {
        return RET_ERR;
    }

    int ret = pthread_mutex_unlock(mtx.lock);
    if (ret == RET_OK) {
        return ret;
    }

    OSAL_ERR("[%s] Mutex unlock failed, %d\n", __FUNCTION__, ret);
    return RET_ERR;
}

/**
 * @fn mutex_destroy
 * @brief destroy mutex, mtx will be deallocated (free)
 *
 * @param mtx
 * @return int
 */
int mutex_destroy(MUTEX_T &mtx) {
    int ret = RET_ERR;

    if (mtx.lock == nullptr) {
        return RET_ERR;
    }

    if ((ret = pthread_mutex_destroy(mtx.lock)) == 0) {
        if (ret == RET_OK) {
            if (strnlen(mtx.mem.name, MTX_NAME_SIZE) > 0) {
                shared_mem_close(mtx.mem);
                shared_mem_destroy(mtx.mem);
            } else {
                if (mtx.lock != nullptr) {
                    delete mtx.lock;
                }
            }
            mtx.lock = nullptr;
        }
    } else {
        OSAL_ERR("errno(%d), mutex is still locked, cannot destroy: %s\n", ret, mtx.name);
    }

    return ret;
}
} // namespace ipc::core