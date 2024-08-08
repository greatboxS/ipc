#include "osal/ipc_file.h"

namespace ipc::core {
int file_exist(const char *file) {
    DWORD dwAttrib = GetFileAttributes(file);
    return static_cast<int>(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int file_is_directory(const char *file) {
    DWORD dwAttrib = GetFileAttributesA(file);
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return 1;
    }
    return RET_OK;
}
} // namespace ipc::core
