#ifndef __TIMER_H__
#define __TIMER_H__

#include "OSAL.h"

namespace gbs
{
    namespace osal
    {
        /**
         * @fn TIMER_Initialize
         * @brief
         *
         * @return int
         */
        __DLL_DECLSPEC__ int TIMER_Initialize();

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
        __DLL_DECLSPEC__ TIMER_T TIMER_Create(int interval, TIMER_Callback fnc, void *param, const char *name = NULL, int start = 1);

        /**
         * @fn TIMER_Start
         * @brief
         *
         * @param timer
         * @return int
         */
        __DLL_DECLSPEC__ int TIMER_Start(TIMER_T &timer);

        /**
         * @fn TIMER_Stop
         * @brief
         *
         * @param timer
         * @return int
         */
        __DLL_DECLSPEC__ int TIMER_Stop(TIMER_T &timer);

        /**
         * @fn TIMER_SetInterval
         * @brief
         *
         * @param timer
         * @param interval
         * @return int
         */
        __DLL_DECLSPEC__ int TIMER_SetInterval(TIMER_T &timer, int interval);

        /**
         * @fn TIMER_Terminate
         * @brief
         *
         * @param timer
         * @return int
         */
        __DLL_DECLSPEC__ int TIMER_Terminate(TIMER_T &timer);
    }; // namespace osal
} // namespace gbs
#endif // __TIMER_H__