#include "cmessage_queue.h"
#include "osal/ipc_message_queue.h"

namespace ipc::core {
int cmessage_queue::open(const char *name) {
    return mesgqueue_open(m_stMsgq, name);
}

int cmessage_queue::create(const char *name, size_t msgsize, size_t msgcount) {
    return mesgqueue_create(m_stMsgq, name, msgsize, msgcount);
}

int cmessage_queue::send(const char *buff, size_t size) {
    return mesgqueue_send(m_stMsgq, buff, size);
}

int cmessage_queue::receive(char *buff, size_t size) {
    return mesgqueue_receive(m_stMsgq, buff, size);
}

int cmessage_queue::size() {
    return mesgqueue_get_current_size(m_stMsgq);
}

int cmessage_queue::close() {
    return mesgqueue_close(m_stMsgq);
}

int cmessage_queue::release() {
    return mesgqueue_destroy(m_stMsgq);
}
} // namespace ipc::core