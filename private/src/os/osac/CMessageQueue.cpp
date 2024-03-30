#include "CMessageQueue.h"
#include "osal/MessageQueue.h"

namespace gbs {
namespace osac {
int CMessageQueue::Open(const char *name) {
    return osal::MSGQ_Open(m_stMsgq, name);
}

int CMessageQueue::Create(const char *name, size_t msgsize, size_t msgcount) {
    return osal::MSGQ_Create(m_stMsgq, name, msgsize, msgcount);
}

int CMessageQueue::Send(const char *buff, size_t size) {
    return osal::MSGQ_Send(m_stMsgq, buff, size);
}

int CMessageQueue::Receive(char *buff, size_t size) {
    return osal::MSGQ_Receive(m_stMsgq, buff, size);
}

int CMessageQueue::Size() {
    return osal::MSGQ_GetCurrSize(m_stMsgq);
}

int CMessageQueue::Close() {
    return osal::MSGQ_Close(m_stMsgq);
}

int CMessageQueue::Release() {
    return osal::MSGQ_Destroy(m_stMsgq);
}
} // namespace osac
} // namespace gbs
