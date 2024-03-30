#ifndef __SECUREDSHAREMEMORY_H__
#define __SECUREDSHAREMEMORY_H__

#include "OSAL.h"
namespace gbs {
namespace osal {
__DLL_DECLSPEC__ SECURED_SHARE_MEMORY_T SECMEM_Create(const char *name, size_t size);
__DLL_DECLSPEC__ int SECMEM_Destroy(SECURED_SHARE_MEMORY_T &secmem);

__DLL_DECLSPEC__ SECURED_SHARE_MEMORY_T SECMEM_Open(const char *name, size_t size);
__DLL_DECLSPEC__ int SECMEM_Close(SECURED_SHARE_MEMORY_T &secmem);

__DLL_DECLSPEC__ int SECMEM_TakeAccess(SECURED_SHARE_MEMORY_T &secmem);
__DLL_DECLSPEC__ int SECMEM_ReleaseAccess(SECURED_SHARE_MEMORY_T &secmem);
__DLL_DECLSPEC__ void *SECMEM_GetBaseAddress(SECURED_SHARE_MEMORY_T &secmem);

__DLL_DECLSPEC__ int SECMEM_Read(SECURED_SHARE_MEMORY_T &secmem, char *buff, size_t size);
__DLL_DECLSPEC__ int SECMEM_Write(SECURED_SHARE_MEMORY_T &secmem, char *buff, size_t size);

/**
 * @fn SECMEM_Seek
 * @brief
 *
 * @param secmem
 * @param pos
 * @param type 0 FILE_BEGIN, 1 FILE_CURRENT, 2 FILE_END
 * @return __DLL_DECLSPEC__
 */
__DLL_DECLSPEC__ int64_t SECMEM_Seek(SECURED_SHARE_MEMORY_T &secmem, int64_t pos, uint32_t type = 0);
__DLL_DECLSPEC__ int64_t SECMEM_CurrentPos(SECURED_SHARE_MEMORY_T &secmem);

}; // namespace osal
} // namespace gbs

#endif // __SECUREDSHAREMEMORY_H__