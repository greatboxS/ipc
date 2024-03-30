#ifndef __THREAD_H__
#define __THREAD_H__

#include "OSAL.h"

namespace gbs {
namespace osal {
/**
 * @fn THREAD_Create
 * @brief Create new thread using pthread library
 *
 * @param name		Thread name
 * @param fnc 		Thread function call
 * @param param 	Parameters
 * @param priority 	Thread priority
 * @param core 		CPU that created thread will be attached to
 * @return TASK_T 	Thread handler ( this TASK_T is allocated by THREAD_Create(), it will be
 * 					deallocated by THREAD_Terminate() )
 */
__DLL_DECLSPEC__ int THREAD_Create(THREAD_T &threadInfo, const char *name, THREAD_FunctionCall fnc, VOID_T param = NULL, int priority = -1, int core = -1);

/**
 * @fn THREAD_Join
 * @brief Wait for thread return
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__DLL_DECLSPEC__ int THREAD_Join(THREAD_T &threadInfo);

/**
 * @fn THREAD_Detach
 * @brief Detach a thread, the resource of this thread automatically
 *        free when application is terminated
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__DLL_DECLSPEC__ int THREAD_Detach(THREAD_T &threadInfo);

/**
 * @fn THREAD_Run
 * @brief
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__DLL_DECLSPEC__ int THREAD_Run(THREAD_T &threadInfo);

/**
 * @fn THREAD_Suspend
 * @brief Suspend thread (WINDOWS)
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__DLL_DECLSPEC__ int THREAD_Suspend(THREAD_T &threadInfo);

/**
 * @fn THREAD_Terminate
 * @brief Terminate a thread intermediately
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__DLL_DECLSPEC__ int THREAD_Terminate(THREAD_T &threadInfo);

/**
 * @fn THREAD_Delay
 * @brief
 *
 * @param ms
 * @param us
 * @return __DLL_DECLSPEC__
 */
__DLL_DECLSPEC__ int THREAD_Delay(unsigned int ms, unsigned int us = 0);
}; // namespace osal
} // namespace gbs
#endif // __THREAD_H__