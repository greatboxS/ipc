#ifndef TIMER_H
#define TIMER_H

#include "osal.h"

namespace ipc::core {
/**
 * @fn timer_initialize
 * @brief
 *
 * @return int
 */
__dll_declspec__ int timer_initialize();

/**
 * @fn timer_create
 * @brief Create Timer using thread base
 *
 * @param interval  Timer interval in ms
 * @param fnc       Callback when timeout
 * @param name      Timer name
 * @param start     Start after create
 * @return TIMER_T
 */
__dll_declspec__ TIMER_T timer_create(int interval, TIMER_Callback fnc, void *param, const char *name = NULL, int start = 1);

/**
 * @fn timer_start
 * @brief
 *
 * @param timer
 * @return int
 */
__dll_declspec__ int timer_start(TIMER_T &timer);

/**
 * @fn timer_stop
 * @brief
 *
 * @param timer
 * @return int
 */
__dll_declspec__ int timer_stop(TIMER_T &timer);

/**
 * @fn timer_set_initerval
 * @brief
 *
 * @param timer
 * @param interval
 * @return int
 */
__dll_declspec__ int timer_set_initerval(TIMER_T &timer, int interval);

/**
 * @fn timer_terminate
 * @brief
 *
 * @param timer
 * @return int
 */
__dll_declspec__ int timer_terminate(TIMER_T &timer);
}
#endif // TIMER_H