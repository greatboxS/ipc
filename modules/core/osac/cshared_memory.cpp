#include "cshared_memory.h"
#include "osal/ipc_shared_memory.h"

namespace ipc::core {
int cshared_memory::open(const char *name, size_t size) { return shared_mem_open(m_stShm, name, size); }

int cshared_memory::create(const char *name, size_t size) { return shared_mem_create(m_stShm, name, size); }

int cshared_memory::close() { return shared_mem_close(m_stShm); }

int cshared_memory::release() { return shared_mem_destroy(m_stShm); }

int cshared_memory::read(char *buff, size_t size) { return shared_mem_read(m_stShm, buff, size); }

int cshared_memory::write(char *buff, size_t size) { return shared_mem_write(m_stShm, buff, size); }

int cshared_memory::seek(int64_t pos, uint32_t type) { return shared_mem_seek(m_stShm, pos, type); }
} // namespace ipc::core