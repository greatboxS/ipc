#ifndef __CFILE_H__
#define __CFILE_H__

#include "osal/OSAL.h"

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CFile
        {
        public:
            static int IsFileExist(const char *file);
            static int IsDirectoryExist(const char *file);
        };
    }; // namespace osac
} // namespace gbs

#endif // __CFILE_H__