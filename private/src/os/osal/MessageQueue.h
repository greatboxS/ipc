#ifndef __MESSAGEQUEUE_H__
#define __MESSAGEQUEUE_H__

#include "OSAL.h"

namespace gbs
{
    namespace osal
    {
        __DLL_DECLSPEC__ int MSGQ_Open(MSGQ_T &msgInfo, const char *name);
        __DLL_DECLSPEC__ int MSGQ_Create(MSGQ_T &msgInfo, const char *name, size_t msgsize, size_t msgcount);
        __DLL_DECLSPEC__ int MSGQ_Receive(MSGQ_T &msgInfo, char *buff, size_t size);
        __DLL_DECLSPEC__ int MSGQ_Send(MSGQ_T &msgInfo, const char *buff, size_t size);
        __DLL_DECLSPEC__ int MSGQ_GetCurrSize(MSGQ_T &msgInfo);
        __DLL_DECLSPEC__ int MSGQ_Close(MSGQ_T &msgInfo);
        __DLL_DECLSPEC__ int MSGQ_Destroy(MSGQ_T &msgInfo);
    }; // namespace osal
} // namespace gbs
#endif // __MESSAGEQUEUE_H__