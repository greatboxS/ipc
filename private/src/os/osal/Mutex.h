#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "OSAL.h"

namespace gbs {
namespace osal {
/**
 * @fn MUTEX_Create
 * @brief 	Create pthread mutex
 *
 * @param mtx		Mutex info structure
 * @param name		Mutex name, using in USE_SHARED_MEMORY_MUTEX type
 * @param recursive Mutex recursive
 * @return MUTEX_T
 */
__DLL_DECLSPEC__ int MUTEX_Create(MUTEX_T &mtx, const char *name = NULL, unsigned int recursive = 0);

/**
 * @fn MUTEX_Lock
 * @brief Mutex lock
 *
 * @param mtx
 * @param timeout 	Wait for mutex available after timeout (ms)
 * @return int
 */
__DLL_DECLSPEC__ int MUTEX_Lock(MUTEX_T &mtx, TIME_T timeout = 0);

/**
 * @fn MUTEX_TryLock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
__DLL_DECLSPEC__ int MUTEX_TryLock(MUTEX_T &mtx);

/**
 * @fn MUTEX_UnLock
 * @brief Mutex unlock
 *
 * @param mtx
 * @return int
 */
__DLL_DECLSPEC__ int MUTEX_UnLock(MUTEX_T &mtx);

/**
 * @fn MUTEX_Destroy
 * @brief Destroy mutex, mtx will be deallocated (free)
 *
 * @param mtx
 * @return int
 */
__DLL_DECLSPEC__ int MUTEX_Destroy(MUTEX_T &mtx);

/**
 * @brief Safe read with mutex lock
 *
 * @tparam T    Object data type
 * @param obj   Object to be writen to
 * @param mtx   Mutex
 * @return T    value of object
 */
template <typename T>
inline __DLL_DECLSPEC__ T MUTEX_SafeRead(T &obj, MUTEX_T &mtx, int *exp = NULL) {
    T ret;
    int err = 0;
    if ((err = MUTEX_Lock(mtx)) >= 0) {
        ret = obj;
        MUTEX_UnLock(mtx);
    } else {
        ret = obj;
    }
    if (exp) *exp = err;
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
inline __DLL_DECLSPEC__ void MUTEX_SafeWrite(T &obj, MUTEX_T &mtx, T val, int *exp = NULL) {
    int err = 0;
    if ((err = MUTEX_Lock(mtx)) >= 0) {
        obj = val;
        MUTEX_UnLock(mtx);
    }
    if (exp) *exp = err;
}
}; // namespace osal
} // namespace gbs
#endif // __MUTEX_H__