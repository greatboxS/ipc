#include "CFile.h"
#include "osal/File.h"

namespace gbs
{
    using namespace osal;
    namespace osac
    {
        int CFile::IsFileExist(const char *file) {
            return FILE_Exist(file);
        }

        int CFile::IsDirectoryExist(const char *file) {
            return FILE_IsDirectory(file);
        }

    } // namespace osac
} // namespace gbs
