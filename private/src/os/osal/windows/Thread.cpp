#include "osal/Thread.h"
#include "dbg/Debug.h"
#include <string.h>

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
int THREAD_Create(THREAD_T &threadInfo, const char *name, THREAD_FunctionCall fnc, VOID_T param, int priority, int core) {
    HANDLE handle;
    DWORD threadId;

    handle = CreateThread(NULL, THREAD_DEFAULT_STACK_SIZE, fnc, param, CREATE_SUSPENDED, &threadId);
    if (handle == NULL) {
        LOG_OSAL_ERROR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    LOG_OSAL_INFO("[%s] Create thread success\n", __FUNCTION__);
    if (SetThreadPriority(handle, priority) == ERROR) {
        LOG_OSAL_ERROR("[%s] SetThreadPriority(), failed %s\n", __FUNCTION__, __ERROR_STR__);
    }

    if (name) {
        int length = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
        WCHAR *buffer = new WCHAR[length];
        MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer, length);
        if (SetThreadDescription(handle, buffer) == ERROR) {
            LOG_OSAL_ERROR("[%s] SetThreadPriority(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
        delete[] buffer;
    }

    if (core >= 0) {
        if (SetThreadAffinityMask(handle, core) == ERROR) {
            LOG_OSAL_ERROR("[%s] SetThreadAffinityMask(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
    }

    LOG_OSAL_INFO("[%s] name: %s, priority: %d, core: %d\n", __FUNCTION__, name, priority, core);
    threadInfo.task = handle;
    threadInfo.core = core;
    threadInfo.priority = priority;
    threadInfo.fnc = fnc;
    if (name)
        strncpy(threadInfo.name, name, sizeof(threadInfo.name));
    return RET_OK;
}

/**
 * @fn THREAD_Join
 * @brief Wait for thread return
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int THREAD_Join(THREAD_T &threadInfo) {
    if (WaitForSingleObject(threadInfo.task, INFINITE) != WAIT_OBJECT_0) {
        LOG_OSAL_ERROR("[%s] WaitForSingleObject(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn THREAD_Detach
 * @brief Detach a thread, the resource of this thread automatically
 *        free when application is terminated
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int THREAD_Detach(THREAD_T &threadInfo) {
    return RET_OK;
}

/**
 * @fn THREAD_Run
 * @brief
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int THREAD_Run(THREAD_T &threadInfo) {

    if (ResumeThread(threadInfo.task) == (DWORD)-1) {
        LOG_OSAL_ERROR("[%s] ResumeThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn THREAD_Suspend
 * @brief Suspend thread (WINDOWS)
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int THREAD_Suspend(THREAD_T &threadInfo) {
    if (SuspendThread(threadInfo.task) == (DWORD)-1) {
        LOG_OSAL_ERROR("[%s] SuspendThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn THREAD_Terminate
 * @brief Terminate a thread intermediately
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int THREAD_Terminate(THREAD_T &threadInfo) {
    if (TerminateThread(threadInfo.task, 0) == ERROR) {
        LOG_OSAL_ERROR("[%s] TerminateThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    CloseHandle(threadInfo.task);
    return RET_OK;
}

int osal::THREAD_Delay(unsigned int ms, unsigned int us) {
    Sleep((DWORD)ms);
    return RET_OK;
}
} // namespace osal
} // namespace gbs