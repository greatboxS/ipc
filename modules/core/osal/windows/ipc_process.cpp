#include "osal/ipc_process.h"
#include "stdio.h"
#include <Windows.h>
#include <signal.h>
#include <string.h>

namespace ipc::core {
int process_create(PROCESS_T &process, char *argv) {

    DWORD flags = CREATE_SUSPENDED;

    if (process.UseExternalConsole) {
        flags |= CREATE_NEW_CONSOLE;
    }
    if (process.s32HighPrioriry) {
        flags |= HIGH_PRIORITY_CLASS;
    }

    memset(&process.stProcessInfo, 0, sizeof(PROCESS_INFORMATION));
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (CreateProcessA(NULL, argv, NULL, NULL, FALSE, flags, NULL, NULL, &si, &process.stProcessInfo) == ERROR) {
        OSAL_ERR("CreateProcess failed (%s).\n", __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("Create child process success\n");
    return RET_OK;
}

int process_execute(PROCESS_T &process, const char *argv) {
    DWORD flags = CREATE_SUSPENDED;

    if (process.UseExternalConsole) {
        flags |= CREATE_NEW_CONSOLE;
    }
    if (process.s32HighPrioriry) {
        flags |= HIGH_PRIORITY_CLASS;
    }

    memset(&process.stProcessInfo, 0, sizeof(PROCESS_INFORMATION));
    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    if (CreateProcessA(NULL, process.argv, NULL, NULL, FALSE, flags, NULL, NULL, &si, &process.stProcessInfo) == ERROR) {
        OSAL_ERR("CreateProcess failed (%s).\n", __ERROR_STR__);
        return RET_ERR;
    }

    OSAL_INFO("Create child process success\n");
    return RET_OK;
}

int process_kill(PROCESS_T &process) {
    if (TerminateProcess(process.stProcessInfo.hProcess, 0) == ERROR) {
        OSAL_ERR("Terminate process failed (%s).\n", __ERROR_STR__);
        return RET_ERR;
    }
    CloseHandle(process.stProcessInfo.hProcess);
    return RET_OK;
}

int process_wait(PROCESS_T &process) {
    if (WaitForSingleObject(process.handle, INFINITE) == ERROR) {
        return RET_ERR;
    }
    return RET_OK;
}

int process_wait_all(PROCESS_T *process[], process_t *handle, size_t size) {
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobExtendedLimitInfo;
    HANDLE hJobObject = NULL;
    hJobObject = CreateJobObject(NULL, NULL);
    *handle = hJobObject;

    if (!hJobObject) {
        OSAL_ERR("Create job object failed\n");
        return RET_ERR;
    }

    if (!AssignProcessToJobObject(hJobObject, GetCurrentProcess())) {
        OSAL_ERR("Can not assign all child process to this Job Object\n");
        CloseHandle(hJobObject);
        return RET_ERR;
    }

    memset(&jobExtendedLimitInfo, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
    memset(&jobExtendedLimitInfo.BasicLimitInformation, 0, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION));
    jobExtendedLimitInfo.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE | JOB_OBJECT_LIMIT_DIE_ON_UNHANDLED_EXCEPTION | JOB_OBJECT_LIMIT_BREAKAWAY_OK;
    if (!SetInformationJobObject(hJobObject, JobObjectExtendedLimitInformation, &jobExtendedLimitInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION))) {
        OSAL_ERR("Failed to set job object information!\n");
        CloseHandle(hJobObject);
        return RET_ERR;
    }

    for (size_t i = 0; i < size; i++) {
        if (!AssignProcessToJobObject(hJobObject, process[i]->stProcessInfo.hProcess)) {
            OSAL_ERR("AssignProcessToJobObject [%s] failed (%s)\n", process[i]->name, __ERROR_STR__);

        } else {
            OSAL_INFO("Resume child process %s\n", process[i]->name);
            ResumeThread(process[i]->stProcessInfo.hThread);
        }
    }

    if (WaitForSingleObject(hJobObject, INFINITE) == WAIT_FAILED) {
        OSAL_ERR("Failed wait for all process!\n");
    }

    for (size_t i = 0; i < size; i++) {
        CloseHandle(process[i]->stProcessInfo.hProcess);
        TerminateProcess(process[i]->stProcessInfo.hProcess, 0);
    }

    TerminateJobObject(hJobObject, 0);
    CloseHandle(hJobObject);
    return RET_OK;
}

int process_get_current_id() {
    return GetCurrentProcessId();
}
} // namespace ipc::core