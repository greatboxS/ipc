#ifndef __OSAL_TIME_H__
#define __OSAL_TIME_H__

#include "common/Typedef.h"

#define TIMESTAMP_T int64_t

namespace gbs
{
    namespace osal
    {
        __DLL_DECLSPEC__ TIMESTAMP_T GetTimeStamp();

    };
} // namespace gbs

#endif // __OSAL_TIME_H__