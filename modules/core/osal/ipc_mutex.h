#ifndef IPC_MUTEX_H
#define IPC_MUTEX_H

#include "osal.h"

namespace ipc::core {
/**
 * @fn mutex_create
 * @brief 	Create pthread mutex
 *
 * @param mtx		Mutex info structure
 * @param name		Mutex name, using in USE_SHARED_MEMORY_MUTEX type
 * @param recursive Mutex recursive
 * @return MUTEX_T
 */
__dll_declspec__ int mutex_create(MUTEX_T &mtx, const char *name = NULL, unsigned int recursive = 0);

/**
 * @fn mutex_lock
 * @brief Mutex lock
 *
 * @param mtx
 * @param timeout 	Wait for mutex available after timeout (ms)
 * @return int
 */
__dll_declspec__ int mutex_lock(MUTEX_T &mtx, TIME_T timeout = 0);

/**
 * @fn mutex_try_lock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
__dll_declspec__ int mutex_try_lock(MUTEX_T &mtx);

/**
 * @fn mutex_unlock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
__dll_declspec__ int mutex_unlock(MUTEX_T &mtx);

/**
 * @fn mutex_destroy
 * @brief destroy mutex, mtx will be deallocated (free)
 *
 * @param mtx
 * @return int
 */
__dll_declspec__ int mutex_destroy(MUTEX_T &mtx);

/**
 * @brief Safe read with mutex lock
 *
 * @tparam T    Object data type
 * @param obj   Object to be writen to
 * @param mtx   Mutex
 * @return T    value of object
 */
template <typename T>
inline __dll_declspec__ T mutex_safe_read(T &obj, MUTEX_T &mtx, int *exp = NULL) {
    T ret;
    int err = 0;
    if ((err = mutex_lock(mtx)) >= 0) {
        ret = obj;
        mutex_unlock(mtx);
    } else {
        ret = obj;
    }
    if (exp) {
        *exp = err;
    }
    return ret;
}

/**
 * @brief Safe write with mutex lock
 *
 * @tparam T    Object data type
 * @param obj   Object to be writen to
 * @param mtx   Mutex
 * @param val   Write value
 */
template <typename T>
inline __dll_declspec__ void mutex_safe_write(T &obj, MUTEX_T &mtx, T val, int *exp = NULL) {
    int err = 0;
    if ((err = mutex_lock(mtx)) >= 0) {
        obj = val;
        mutex_unlock(mtx);
    }
    if (exp) {
        *exp = err;
    }
}
}
#endif // IPC_MUTEX_H