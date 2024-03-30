#include "CSemaphore.h"
#include "Semaphore.h"
#include <string.h>

namespace gbs {
namespace osac {
CSemaphore::CSemaphore() { memset(&m_stSem, 0, sizeof(SEM_T)); }

CSemaphore::~CSemaphore() {}

int CSemaphore::Create(int value, const char *name) { return osal::SEM_Create(m_stSem, value, name); }

int CSemaphore::Wait() { return osal::SEM_Wait(m_stSem); }

int CSemaphore::Post() { return osal::SEM_Post(m_stSem); }

int CSemaphore::Value() { return osal::SEM_Value(m_stSem); }

int CSemaphore::Close() { return osal::SEM_Close(m_stSem); }

int CSemaphore::Destroy() { return osal::SEM_Destroy(m_stSem); }
} // namespace osac
} // namespace gbs