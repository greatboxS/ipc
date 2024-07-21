#ifndef IPC_SEMAPHORE_H
#define IPC_SEMAPHORE_H

#include "osal.h"

namespace ipc::core {
/**
 * @fn semaphore_open
 * @brief Open named sem (LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @return SEM_T    Pointer to opened semaphore
 */
__dll_declspec__ int semaphore_open(SEM_T &sem, const char *name);

/**
 * @fn semaphore_create
 * @brief Create (named sem LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name, if name is null, the unname semaphore will be created
 * @param value     is_initialized value
 * @return SEM_T    Pointer to created semaphore
 */
__dll_declspec__ int semaphore_create(SEM_T &sem, int value, const char *name = NULL);

/**
 * @fn semaphore_wait
 * @brief Decrease sem 1 value (wait)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
__dll_declspec__ int semaphore_wait(SEM_T &sem);

/**
 * @fn semaphore_post
 * @brief Increase semaphore 1 value (release)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
__dll_declspec__ int semaphore_post(SEM_T &sem);

/**
 * @fn semaphore_value
 * @brief Get semaphore value
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
__dll_declspec__ int semaphore_value(SEM_T &sem);

/**
 * @fn semaphore_close
 * @brief Close semaphore
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
__dll_declspec__ int semaphore_close(SEM_T &sem);

/**
 * @fn semaphore_destroy
 * @brief destroy semaphore
 *
 * @param sem		Semaphore info structure
 *
 * @return int  0 if success, otherwise -1
 */
__dll_declspec__ int semaphore_destroy(SEM_T &sem);
}
#endif // IPC_SEMAPHORE_H