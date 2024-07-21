#include "csemaphore.h"
#include "osal/ipc_semaphore.h"
#include <string.h>

namespace ipc::core {
csemaphore::csemaphore() { memset(&m_stSem, 0, sizeof(SEM_T)); }

csemaphore::~csemaphore() {}

int csemaphore::create(int value, const char *name) { return semaphore_create(m_stSem, value, name); }

int csemaphore::wait() { return semaphore_wait(m_stSem); }

int csemaphore::post() { return semaphore_post(m_stSem); }

int csemaphore::value() { return semaphore_value(m_stSem); }

int csemaphore::close() { return semaphore_close(m_stSem); }

int csemaphore::destroy() { return semaphore_destroy(m_stSem); }
} // namespace ipc::core