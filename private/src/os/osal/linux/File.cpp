#include "osal/File.h"
#include <sys/stat.h>

namespace gbs
{
    namespace osal
    {
        int FILE_Exist(const char *file) {
            struct stat statbuf;
            if (stat(file, &statbuf) != 0)
                return RET_OK;
            return S_ISREG(statbuf.st_mode);
        }

        int FILE_IsDirectory(const char *file) {
            struct stat statbuf;
            if (stat(file, &statbuf) != 0)
                return RET_OK;
            return S_ISDIR(statbuf.st_mode);
        }
    } // namespace osal
} // namespace gbs
