#include "osal/Process.h"
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>

namespace gbs
{
    namespace osal
    {
        int PROCESS_Create(PROCESS_T &process, char *path) {
            if (path) {
                strncpy(process.name, path, sizeof(process.name));
            }
            pid_t pid = fork();
            process.handle = pid;
            return pid;
        }

        int PROCESS_Execute(PROCESS_T &process, const char *argv) {
            pid_t pid = 0;
            if ((pid = fork()) == 0)
                return system(argv);
            else if (pid < 0)
                return RET_ERR;
            else
                return RET_OK;
        }

        int PROCESS_Kill(PROCESS_T &process) {
            return kill(process.handle, SIGKILL);
        }

        int PROCESS_Wait(PROCESS_T &process) {
            return waitpid(process.handle, NULL, 0);
        }

        int PROCESS_WaitAll(PROCESS_T *process[], process_t *handle, size_t size) {
            while (wait(NULL) > 0) {}
            return RET_OK;
        };

        int PROCESS_GetCurrentId() {
            return getpid();
        }
    } // namespace osal
} // namespace gbs