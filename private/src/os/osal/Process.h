#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "OSAL.h"

namespace gbs {
namespace osal {
__DLL_DECLSPEC__ int PROCESS_Create(PROCESS_T &process, char *argv = NULL);
__DLL_DECLSPEC__ int PROCESS_Execute(PROCESS_T &process, const char *cmd);
__DLL_DECLSPEC__ int PROCESS_Kill(PROCESS_T &process);
__DLL_DECLSPEC__ int PROCESS_Wait(PROCESS_T &process);
__DLL_DECLSPEC__ int PROCESS_WaitAll(PROCESS_T *process[], process_t *handle, size_t size);
__DLL_DECLSPEC__ int PROCESS_GetCurrentId();
}; // namespace osal
} // namespace gbs
#endif // __PROCESS_H__