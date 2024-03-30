#ifndef __FILE_H__
#define __FILE_H__

#include "OSAL.h"

namespace gbs
{
    namespace osal
    {
        __DLL_DECLSPEC__ int FILE_Open(FILE_T &file, const char *name, int mode);
        __DLL_DECLSPEC__ int FILE_Create(FILE_T &file, const char *name);
        __DLL_DECLSPEC__ int FILE_Close(FILE_T &file);
        __DLL_DECLSPEC__ int FILE_Seek(FILE_T &file, long pos);
        __DLL_DECLSPEC__ int FILE_Read(FILE_T &file, char *buff, size_t size);
        __DLL_DECLSPEC__ int FILE_Write(FILE_T &file, const char *buff, size_t size);

        __DLL_DECLSPEC__ int FILE_Exist(const char *file);
        __DLL_DECLSPEC__ int FILE_IsDirectory(const char *file);
    }; // namespace osal
} // namespace gbs
#endif // __FILE_H__