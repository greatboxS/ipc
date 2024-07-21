#include "osal/ipc_thread.h"
#include <string.h>

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
int thread_create(THREAD_T &threadInfo, const char *name, THREAD_FunctionCall fnc, VOID_T param, int priority, int core) {
    HANDLE handle;
    DWORD threadId;

    handle = CreateThread(NULL, THREAD_DEFAULT_STACK_SIZE, fnc, param, CREATE_SUSPENDED, &threadId);
    if (handle == NULL) {
        OSAL_ERR("[%s] failed, %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("[%s] Create thread success\n", __FUNCTION__);
    if (SetThreadPriority(handle, priority) == ERROR) {
        OSAL_ERR("[%s] SetThreadPriority(), failed %s\n", __FUNCTION__, __ERROR_STR__);
    }

    if (name) {
        int length = MultiByteToWideChar(CP_UTF8, 0, name, -1, NULL, 0);
        WCHAR *buffer = new WCHAR[length];
        MultiByteToWideChar(CP_UTF8, 0, name, -1, buffer, length);
        if (SetThreadDescription(handle, buffer) == ERROR) {
            OSAL_ERR("[%s] SetThreadPriority(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
        delete[] buffer;
    }

    if (core >= 0) {
        if (SetThreadAffinityMask(handle, core) == ERROR) {
            OSAL_ERR("[%s] SetThreadAffinityMask(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
    }

    OSAL_INFO("[%s] name: %s, priority: %d, core: %d\n", __FUNCTION__, name, priority, core);
    threadInfo.task = handle;
    threadInfo.core = core;
    threadInfo.priority = priority;
    threadInfo.fnc = fnc;
    if (name)
        strncpy(threadInfo.name, name, sizeof(threadInfo.name));
    return RET_OK;
}

/**
 * @fn thread_join
 * @brief Wait for thread return
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_join(THREAD_T &threadInfo) {
    if (WaitForSingleObject(threadInfo.task, INFINITE) != WAIT_OBJECT_0) {
        OSAL_ERR("[%s] WaitForSingleObject(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn thread_detach
 * @brief Detach a thread, the resource of this thread automatically
 *        free when application is terminated
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_detach(THREAD_T &threadInfo) {
    return RET_OK;
}

/**
 * @fn thread_run
 * @brief
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_run(THREAD_T &threadInfo) {

    if (ResumeThread(threadInfo.task) == (DWORD)-1) {
        OSAL_ERR("[%s] ResumeThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn thread_suspend
 * @brief Suspend thread (WINDOWS)
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_suspend(THREAD_T &threadInfo) {
    if (SuspendThread(threadInfo.task) == (DWORD)-1) {
        OSAL_ERR("[%s] SuspendThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
}

/**
 * @fn thread_terminate
 * @brief Terminate a thread intermediately
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_terminate(THREAD_T &threadInfo) {
    if (TerminateThread(threadInfo.task, 0) == ERROR) {
        OSAL_ERR("[%s] TerminateThread(), failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    CloseHandle(threadInfo.task);
    return RET_OK;
}

int osal::thread_delay(unsigned int ms, unsigned int us) {
    Sleep((DWORD)ms);
    return RET_OK;
}
} // namespace ipc::core