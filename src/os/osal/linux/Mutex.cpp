#include "osal/Mutex.h"
#include "dbg/Debug.h"
#include "osal/SharedMemory.h"
#include <string.h>

namespace gbs
{
    namespace osal
    {

#define GENERATE_MUTEX_NAME(from)        \
    char genName[MTX_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "%s_mtx", from);

        /**
         * @fn MUTEX_Create
         * @brief 	Create pthread mutex
         *
         * @param mtx		Mutex info structure
         * @param name	Mutex name, using in USE_SHARED_MEMORY_MUTEX type
         * @param recursive Mutex recursive
         * @return int
         */
        int MUTEX_Create(MUTEX_T &mtx, const char *name, unsigned int recursive) {
            pthread_mutexattr_t attr;
            int ret = 0;

            memset(&mtx, 0, sizeof(MUTEX_t));

            if (name) {
                GENERATE_MUTEX_NAME(name);

                if (SHM_Create(mtx.mem, genName, sizeof(pthread_mutex_t)) != RET_OK) {
                    LOG_OSAL_ERROR("Create pthread_mutex_t with shared memory failed\n");
                    return RET_ERR;
                }
                snprintf(mtx.mem.name, sizeof(mtx.mem.name), "%s", genName);
                // Use shared memory to allocate MTX
                mtx.lock = (pthread_mutex_t *)mtx.mem.virt;
            } else {
                mtx.lock = new pthread_mutex_t;
                assert(mtx.lock);
            }
            if (pthread_mutexattr_init(&attr) != RET_OK) {
                LOG_OSAL_ERROR("pthread_mutex_attr_init() error (%s)", __ERROR_STR__);
                goto exit_error;
            }

            /**
             * The mutex can be shared with other thread that has the access
             * to this mutex, even if the mutex is allocated in memory that is
             * shared by multiple processes
             */
            if ((ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) != RET_OK) {
                LOG_OSAL_ERROR("pthread_mutexattr_setpshared() error %s\n", __ERROR_STR__);
            }

            /* Mutex recusive, which can be unlock when no lock is called*/
            if (recursive > 0) {
                if ((ret = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != RET_OK) {
                    LOG_OSAL_ERROR("pthread_mutexattr_settype() error %s\n", __ERROR_STR__);
                }
            }
            /* Mutex attr protocol set */
            if ((ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT)) != RET_OK) {
                LOG_OSAL_ERROR("pthread_mutexattr_setprotocol() error %s\n", __ERROR_STR__);
            }
            /* Mutex init */
            if ((ret = pthread_mutex_init(mtx.lock, &attr)) != RET_OK) {
                LOG_OSAL_ERROR("pthread_mutex_init() error %s\n", __ERROR_STR__);
                pthread_mutexattr_destroy(&attr);
                goto exit_error;
            }
            /* Destroy initialized mutex attribute */
            if ((ret = pthread_mutexattr_destroy(&attr)) != RET_OK) {
                LOG_OSAL_ERROR("pthread_mutexattr_destroy() error %s\n", __ERROR_STR__);
            }
            LOG_OSAL_INFO("[%s] Create mutex %s success\n", __FUNCTION__, name);
            return RET_OK;

        exit_error:
            if (name) {
                SHM_Destroy(mtx.mem);
                SHM_Close(mtx.mem);
                return RET_ERR;
            }

            delete mtx.lock;
            return RET_ERR;
        }

        /**
         * @fn MUTEX_Lock
         * @brief Mutex lock
         *
         * @param mtx
         * @param timeout 	Wait for mutex available after timeout (ms)
         * @return int
         */
        int MUTEX_Lock(MUTEX_T &mtx, long int timeout) {
            int ret = 0;
            if (timeout > 0) {
                timespec time;
                long int sec = (int)(timeout / 1000.0f);
                long int nano = (timeout - sec * 1000) * 1000;
                time.tv_sec = sec;
                time.tv_nsec = nano;
                if ((ret = pthread_mutex_timedlock(mtx.lock, &time)) != RET_OK) {
                    LOG_OSAL_ERROR("[%s] Mutex timelock failed, %d\n", __FUNCTION__, ret);
                    return RET_ERR;
                }
            }

            ret = pthread_mutex_lock(mtx.lock);
            if (ret == RET_OK) return RET_OK;

            LOG_OSAL_ERROR("[%s] Mutex lock failed, %d\n", __FUNCTION__, ret);
            return RET_ERR;
        }

        /**
         * @fn MUTEX_TryLock
         * @brief Mutex unlock
         *
         * @param mtx
         * @return int
         */
        int MUTEX_TryLock(MUTEX_T &mtx) {
            int ret = pthread_mutex_trylock(mtx.lock);
            if (ret == RET_OK) return ret;

            LOG_OSAL_ERROR("[%s] Mutex trylock failed, %d\n", __FUNCTION__, ret);
            return RET_ERR;
        }

        /**
         * @fn MUTEX_UnLock
         * @brief Mutex unlock
         *
         * @param mtx
         * @return int
         */
        int MUTEX_UnLock(MUTEX_T &mtx) {
            int ret = pthread_mutex_unlock(mtx.lock);
            if (ret == RET_OK) return ret;

            LOG_OSAL_ERROR("[%s] Mutex unlock failed, %d\n", __FUNCTION__, ret);
            return RET_ERR;
        }

        /**
         * @fn MUTEX_Destroy
         * @brief Destroy mutex, mtx will be deallocated (free)
         *
         * @param mtx
         * @return int
         */
        int MUTEX_Destroy(MUTEX_T &mtx) {
            int ret = pthread_mutex_destroy(mtx.lock);
            if (strlen(mtx.mem.name) != RET_OK) {
                SHM_Close(mtx.mem);
                SHM_Destroy(mtx.mem);
                return RET_OK;
            }

            delete mtx.lock;
            return RET_ERR;
        }
    } // namespace osal
} // namespace gbs