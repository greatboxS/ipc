#include "osal/Timer.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include "osal/Thread.h"

#include <chrono>
#include <string.h>
#include <string>
#include <vector>

namespace gbs
{
    namespace osal
    {
#define TIMER_TICK        1000
#define clock_now()       std::chrono::high_resolution_clock::now()
#define clock_duration(x) std::chrono::duration_cast<std::chrono::microseconds>(x)

        struct TIMER_Wrapper_t {
            TIMER_T timer;
            int tick;
            int start;
        };

        struct TIMER_Manager_t {
            MUTEX_t mtx;
            std::vector<TIMER_Wrapper_t> nTimers;
        };

        /* TIMER MANAGER OBJECTS */
        static THREAD_T mTimerThread;
        static TIMER_Manager_t TIMER_Manager;
        static int mInitialized = 0;
        static int mTimerIndex = 0;
        static auto mStartTime = clock_now();

        /**
         * @fn TIMER_Running
         * @brief Timer thread loop base
         *
         * @param param
         * @return void*
         */
        static TASK_RET TIMER_Running(TASK_ARG param) {
            LOG_OSAL_INFO("%s\n", __FUNCTION__);
            while (true) {

#if defined(WIN32) || defined(_WIN32)
                while (clock_duration(clock_now() - mStartTime).count() < TIMER_TICK) {}
                mStartTime = clock_now();
#else
                osal::THREAD_Delay(1);
#endif

                MUTEX_Lock(TIMER_Manager.mtx);
                for (TIMER_Wrapper_t &wrapper : TIMER_Manager.nTimers) {
                    if (!wrapper.start) continue;
                    wrapper.tick++;
                    if (wrapper.tick >= wrapper.timer.interval) {
                        wrapper.tick = 0;
                        if (wrapper.timer.fnc) wrapper.timer.fnc(wrapper.timer.param);
                    }
                }
                MUTEX_UnLock(TIMER_Manager.mtx);
            }
            return NULL;
        }

        /**
         * @fn TIMER_Initialize
         * @brief Initialize timer loop
         *
         * @return int
         */
        int TIMER_Initialize() {
            LOG_OSAL_INFO("%s\n", __FUNCTION__);
            if (MUTEX_Create(TIMER_Manager.mtx) < 0) {
                LOG_OSAL_ERROR("%s: Failed to create Mutex failed\n", __FUNCTION__);
                return RET_ERR;
            }

            if (THREAD_Create(mTimerThread, "MainTimerThread", TIMER_Running, &TIMER_Manager, 0) != 0) {
                LOG_OSAL_ERROR("%s: Failed to create MainTimerThread\n", __FUNCTION__);
                return RET_ERR;
            }
            THREAD_Run(mTimerThread);
            THREAD_Detach(mTimerThread);
            return RET_OK;
        }

        /**
         * @fn TIMER_Create
         * @brief Create Timer using thread base
         *
         * @param interval  Timer interval in ms
         * @param fnc       Callback when timeout
         * @param name      Timer name
         * @param start     Start after create
         * @return TIMER_T
         */
        TIMER_T TIMER_Create(int interval, TIMER_Callback fnc, void *param, const char *name, int start) {
            TIMER_Wrapper_t wrapper;

            memset(&wrapper, 0, sizeof(wrapper));

            if (mInitialized == 0) {
                if (TIMER_Initialize() < 0)
                    return wrapper.timer;
                else
                    mInitialized = 1;
            }

            if (interval <= 0 || !fnc) {
                _EXCEPT_THROW("TIMER_Create error, invalid arguments!");
            }

            wrapper.tick = 0;
            wrapper.start = start;
            wrapper.timer.fnc = fnc;
            wrapper.timer.interval = interval;
            wrapper.timer.param = param;
            wrapper.timer.id = ++mTimerIndex;

            if (name) strncpy(wrapper.timer.name, name, sizeof(wrapper.timer.name));

            MUTEX_Lock(TIMER_Manager.mtx);
            TIMER_Manager.nTimers.push_back(wrapper);
            MUTEX_UnLock(TIMER_Manager.mtx);

            LOG_OSAL_INFO("[%s] Create new Timer %d\n", __FUNCTION__, wrapper.timer.id);
            return wrapper.timer;
        }

        /**
         * @fn TIMER_Start
         * @brief
         *
         * @param timer
         * @return int
         */
        int TIMER_Start(TIMER_T &timer) {
            int ret = -1;
            MUTEX_Lock(TIMER_Manager.mtx);
            for (auto &wrapper : TIMER_Manager.nTimers) {
                if (timer.id == wrapper.timer.id) {
                    wrapper.start = 1;
                    ret = 0;
                    break;
                }
            }
            MUTEX_UnLock(TIMER_Manager.mtx);
            return ret;
        }

        /**
         * @fn TIMER_Stop
         * @brief
         *
         * @param timer
         * @return int
         */
        int TIMER_Stop(TIMER_T &timer) {
            int ret = -1;
            MUTEX_Lock(TIMER_Manager.mtx);
            for (auto &wrapper : TIMER_Manager.nTimers) {
                if (timer.id == wrapper.timer.id) {
                    wrapper.start = 0;
                    ret = 0;
                    break;
                }
            }
            MUTEX_UnLock(TIMER_Manager.mtx);
            return ret;
        }

        /**
         * @fn TIMER_SetInterval
         * @brief
         *
         * @param timer
         * @param interval
         * @return int
         */
        int TIMER_SetInterval(TIMER_T &timer, int interval) {
            int ret = -1;
            MUTEX_Lock(TIMER_Manager.mtx);
            for (auto &wrapper : TIMER_Manager.nTimers) {
                if (timer.id == wrapper.timer.id) {
                    wrapper.timer.interval = interval;
                    break;
                }
            }
            MUTEX_UnLock(TIMER_Manager.mtx);
            return ret;
        }

        /**
         * @fn TIMER_Terminate
         * @brief
         *
         * @param timer
         * @return int
         */
        int TIMER_Terminate(TIMER_T &timer) {
            int ret = -1;
            MUTEX_Lock(TIMER_Manager.mtx);
            for (auto i = TIMER_Manager.nTimers.begin(); i != TIMER_Manager.nTimers.end(); i++) {
                if (timer.id == i->timer.id) {
                    TIMER_Manager.nTimers.erase(i);
                    break;
                }
            }
            MUTEX_UnLock(TIMER_Manager.mtx);
            return ret;
        }
    } // namespace osal
} // namespace gbs