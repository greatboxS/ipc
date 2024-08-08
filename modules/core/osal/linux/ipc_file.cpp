#include "osal/ipc_file.h"
#include <sys/stat.h>

namespace ipc::core {
int file_exist(const char *file) {
    struct stat statbuf;
    if (stat(file, &statbuf) != 0) {
        return RET_OK;
    }
    return S_ISREG(statbuf.st_mode);
}

int file_is_directory(const char *file) {
    struct stat statbuf;
    if (stat(file, &statbuf) != 0) {
        return RET_OK;
    }
    return S_ISDIR(statbuf.st_mode);
}
} // namespace ipc::core
