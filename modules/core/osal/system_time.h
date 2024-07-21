#ifndef __OSAL_TIME_H__
#define __OSAL_TIME_H__

#include "osal.h"

#define TIMESTAMP_T int64_t

namespace ipc::core {
__dll_declspec__ TIMESTAMP_T get_timestamp();

} // namespace gbs

#endif // __OSAL_TIME_H__