#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "OSAL.h"

namespace gbs {
namespace osal {
/**
 * @fn SEM_Open
 * @brief Open named sem (LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name
 * @return SEM_T    Pointer to opened semaphore
 */
__DLL_DECLSPEC__ int SEM_Open(SEM_T &sem, const char *name);

/**
 * @fn SEM_Create
 * @brief Create (named sem LINUX) or WINDOWS
 *
 * @param sem		Semaphore info structure
 * @param name      Semaphore name, if name is null, the unname semaphore will be created
 * @param value     Initialized value
 * @return SEM_T    Pointer to created semaphore
 */
__DLL_DECLSPEC__ int SEM_Create(SEM_T &sem, int value, const char *name = NULL);

/**
 * @fn SEM_Wait
 * @brief Decrease sem 1 value (wait)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
__DLL_DECLSPEC__ int SEM_Wait(SEM_T &sem);

/**
 * @fn SEM_Post
 * @brief Increase semaphore 1 value (release)
 *
 * @param sem		Semaphore info structure
 * @return int
 */
__DLL_DECLSPEC__ int SEM_Post(SEM_T &sem);

/**
 * @fn SEM_Value
 * @brief Get semaphore value
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
__DLL_DECLSPEC__ int SEM_Value(SEM_T &sem);

/**
 * @fn SEM_Close
 * @brief Close semaphore
 *
 * @param sem		Semaphore info structure
 * @return int      0 if success, otherwise -1
 */
__DLL_DECLSPEC__ int SEM_Close(SEM_T &sem);

/**
 * @fn SEM_Destroy
 * @brief Destroy semaphore
 *
 * @param sem		Semaphore info structure
 *
 * @return int  0 if success, otherwise -1
 */
__DLL_DECLSPEC__ int SEM_Destroy(SEM_T &sem);
}; // namespace osal
} // namespace gbs
#endif // __SEMAPHORE_H__