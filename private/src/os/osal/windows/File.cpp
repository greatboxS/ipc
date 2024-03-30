#include "osal/File.h"

namespace gbs {
namespace osal {
int FILE_Exist(const char *file) {
    DWORD dwAttrib = GetFileAttributes(file);
    return static_cast<int>(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int FILE_IsDirectory(const char *file) {
    DWORD dwAttrib = GetFileAttributesA(file);
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)
        return 1;
    return RET_OK;
}
} // namespace osal
} // namespace gbs
