#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include "osal.h"

namespace ipc::core {
__dll_declspec__ int mesgqueue_open(MSGQ_T &msgInfo, const char *name);
__dll_declspec__ int mesgqueue_create(MSGQ_T &msgInfo, const char *name, size_t msgsize, size_t msgcount);
__dll_declspec__ int mesgqueue_receive(MSGQ_T &msgInfo, char *buff, size_t size);
__dll_declspec__ int mesgqueue_send(MSGQ_T &msgInfo, const char *buff, size_t size);
__dll_declspec__ int mesgqueue_get_current_size(MSGQ_T &msgInfo);
__dll_declspec__ int mesgqueue_close(MSGQ_T &msgInfo);
__dll_declspec__ int mesgqueue_destroy(MSGQ_T &msgInfo);
}
#endif // IPC_MESSAGE_QUEUE_H