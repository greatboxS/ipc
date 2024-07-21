#ifndef IPC_THREAD_H
#define IPC_THREAD_H

#include "osal.h"

namespace ipc::core {
/**
 * @fn thread_create
 * @brief Create new thread using pthread library
 *
 * @param name		Thread name
 * @param fnc 		Thread function call
 * @param param 	Parameters
 * @param priority 	Thread priority
 * @param core 		CPU that created thread will be attached to
 * @return TASK_T 	Thread handler ( this TASK_T is allocated by thread_create(), it will be
 * 					deallocated by thread_terminate() )
 */
__dll_declspec__ int thread_create(THREAD_T &threadInfo, const char *name, THREAD_FunctionCall fnc, VOID_T param = NULL, int priority = -1, int core = -1);

/**
 * @fn thread_join
 * @brief Wait for thread return
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__dll_declspec__ int thread_join(THREAD_T &threadInfo);

/**
 * @fn thread_detach
 * @brief Detach a thread, the resource of this thread automatically
 *        free when application is terminated
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__dll_declspec__ int thread_detach(THREAD_T &threadInfo);

/**
 * @fn thread_run
 * @brief
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__dll_declspec__ int thread_run(THREAD_T &threadInfo);

/**
 * @fn thread_suspend
 * @brief Suspend thread (WINDOWS)
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__dll_declspec__ int thread_suspend(THREAD_T &threadInfo);

/**
 * @fn thread_terminate
 * @brief Terminate a thread intermediately
 *
 * @param threadInfo Thread info structure
 * @return int
 */
__dll_declspec__ int thread_terminate(THREAD_T &threadInfo);

/**
 * @fn thread_delay
 * @brief
 *
 * @param ms
 * @param us
 * @return __dll_declspec__
 */
__dll_declspec__ int thread_delay(unsigned int ms, unsigned int us = 0);
}
#endif // IPC_THREAD_H