#include "osal/Semaphore.h"
#include "dbg/Debug.h"
#include <Windows.h>
#include <stdio.h>
#include <string.h>

namespace gbs
{
    namespace osal
    {

#define GENERATE_SEM_NAME(from)          \
    char genName[SEM_NAME_SIZE + 4];     \
    memset(genName, 0, sizeof(genName)); \
    snprintf(genName, sizeof(genName), "sem.%s", from);

        /**
         * @fn SEM_Open
         * @brief Open named sem (LINUX) or WINDOWS
         *
         * @param name      Semaphore name
         * @return int
         */
        int SEM_Open(SEM_T &sem, const char *name) {
            int ret = RET_ERR;
            if (name) {
                GENERATE_SEM_NAME(name);
                strncpy(sem.name, name, sizeof(sem.name));
                HANDLE handle = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, genName);
                if (handle != NULL) {
                    LOG_OSAL_INFO("[%s] Open semaphore %s success\n", __FUNCTION__, name);
                    ret = RET_OK;
                    sem.handle = handle;
                } else {
                    LOG_OSAL_ERROR("[%s] Open semaphore %s failed, %s\n", __FUNCTION__, name, __ERROR_STR__);
                }
            }
            return ret;
        }

        /**
         * @fn SEM_Create
         * @brief Create (named sem LINUX) or WINDOWS
         *
         * @param name      Semaphore name
         * @param value     Initialized value
         * @return SEM_T    Pointer to created semaphore
         */
        int SEM_Create(SEM_T &sem, int value, const char *name) {
            HANDLE handle = NULL;
            SECURITY_DESCRIPTOR sd;
            SECURITY_ATTRIBUTES sa;

            assert(InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION));
            assert(SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE));
            sa.nLength = sizeof(SECURITY_ATTRIBUTES);
            sa.lpSecurityDescriptor = &sd;
            sa.bInheritHandle = FALSE;

            if (!name) {
                if ((handle = CreateSemaphoreA(NULL, value, value, NULL)) == NULL) {
                    LOG_OSAL_ERROR("[%s] Create semaphore %s failed, %s\n", __FUNCTION__, name, __ERROR_STR__);
                    return RET_ERR;
                }

                LOG_OSAL_INFO("[%s] Create semaphore %s success\n", __FUNCTION__, name, __ERROR__);
                sem.handle = handle;
                return RET_OK;
            }

            if (SEM_Open(sem, name) == RET_OK) return RET_OK;

            strncpy(sem.name, name, sizeof(sem.name));
            GENERATE_SEM_NAME(name);

            if (__ERROR__ == ERROR_FILE_NOT_FOUND) {
                if ((handle = CreateSemaphoreA(&sa, value, value, genName)) == NULL) {
                    LOG_OSAL_ERROR("[%s] Create semaphore failed, %s\n", __FUNCTION__, __ERROR_STR__);
                    return RET_ERR;
                }

                LOG_OSAL_INFO("[%s] Create semaphore %s success\n", __FUNCTION__, name);
                sem.handle = handle;
            }
            return RET_OK;
        }

        /**
         * @fn SEM_Wait
         * @brief Decrease sem 1 value (wait)
         *
         * @param sem
         * @return int
         */
        int SEM_Wait(SEM_T &sem) {
            int ret = WaitForSingleObject(sem.handle, INFINITE);
            if (ret != WAIT_OBJECT_0) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
        }

        /**
         * @fn SEM_Post
         * @brief Increase semaphore 1 value (release)
         *
         * @param sem
         * @return int
         */
        int SEM_Post(SEM_T &sem) {
            int ret = ReleaseSemaphore(sem.handle, 1, NULL);
            if (ret == ERROR) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
        }

        /**
         * @fn SEM_Value
         * @brief Get current semaphore value
         *
         * @param sem		Semaphore info structure
         * @return int      value if success, otherwise -1
         */
        int SEM_Value(SEM_T &sem) {
            LONG count = -1;
            if (WAIT_OBJECT_0 == WaitForSingleObject(sem.handle, 0L)) {
                ReleaseSemaphore(sem.handle, 1, &count);
            }
            return (int)count;
        }

        /**
         * @fn SEM_Close
         * @brief Close semaphore
         *
         * @param sem       Semaphore to be closed
         * @return int      0 if success, otherwise -1
         */
        int SEM_Close(SEM_T &sem) {
            int ret = CloseHandle(sem.handle);
            if (ret == ERROR) {
                LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
                return RET_ERR;
            }
            return RET_OK;
        }

        /**
         * @fn SEM_Destroy
         * @brief Destroy semaphore
         *
         * @param sem   Semaphore to be destroy [linux - unnamed semaphore] or [Windows]
         * @param name  If name is not NULL, this fnc use to unlink named semaphore (linux)
         *              If name is NULL, this fnc using to destroy unnamed semaphore (linux)
         *
         * @return int  0 if success, otherwise -1
         */
        int SEM_Destroy(SEM_T &sem) {
            return RET_OK;
        }
    } // namespace osal
} // namespace gbs
