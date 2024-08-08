#include "osal/ipc_process.h"
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

namespace ipc::core {
int process_create(PROCESS_T &process, char *path) {
    if (path) {
        strncpy(process.name, path, sizeof(process.name));
    }
    pid_t pid = fork();
    process.handle = pid;
    return pid;
}

int process_execute(PROCESS_T &process, const char *argv) {
    pid_t pid = 0;
    if ((pid = fork()) == 0) {
        return system(argv);
    }
    else if (pid < 0) {
        return RET_ERR;
    }
    else {
        return RET_OK;
    }
}

int process_kill(PROCESS_T &process) {
    return kill(process.handle, SIGKILL);
}

int process_wait(PROCESS_T &process) {
    return waitpid(process.handle, NULL, 0);
}

int process_wait_all(PROCESS_T *process[], process_t *handle, size_t size) {
    while (wait(NULL) > 0) {}
    return RET_OK;
};

int process_get_current_id() {
    return getpid();
}
} // namespace ipc::core