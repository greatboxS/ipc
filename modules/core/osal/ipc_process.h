#ifndef IPC_PROCESS_H
#define IPC_PROCESS_H

#include "osal.h"

namespace ipc::core {
__dll_declspec__ int process_create(PROCESS_T &process, char *argv = NULL);
__dll_declspec__ int process_execute(PROCESS_T &process, const char *cmd);
__dll_declspec__ int process_kill(PROCESS_T &process);
__dll_declspec__ int process_wait(PROCESS_T &process);
__dll_declspec__ int process_wait_all(PROCESS_T *process[], process_t *handle, size_t size);
__dll_declspec__ int process_get_current_id();
}
#endif // IPC_PROCESS_H