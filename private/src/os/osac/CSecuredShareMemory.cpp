#include "CSecuredShareMemory.h"
#include "osal/SecuredShareMemory.h"
#include <string.h>
#include <random>

namespace gbs {
namespace osac {
using namespace osal;
CSecuredShareMemory::CSecuredShareMemory(const char *name, size_t size) {
    if (!name) {
        sprintf(m_strName, "securedSHM%d", rand());
    } else
        strncpy(m_strName, name, SHM_NAME_SIZE);
    m_size = size;
}

CSecuredShareMemory::~CSecuredShareMemory() {
    if (m_stSecMem.isopen) {
        Close();
        Destroy();
    }
}

int CSecuredShareMemory::Create() {
    m_stSecMem = SECMEM_Create(m_strName, m_size);
    return (m_stSecMem.isopen == 0) ? -1 : 0;
}

int CSecuredShareMemory::Destroy() { return SECMEM_Destroy(m_stSecMem); }

int CSecuredShareMemory::Open() {
    m_stSecMem = SECMEM_Open(m_strName, m_size);
    return (m_stSecMem.isopen == 0) ? -1 : 0;
}

int CSecuredShareMemory::Close() { return SECMEM_Close(m_stSecMem); }

int CSecuredShareMemory::IsOpen() { return m_stSecMem.isopen; }

int CSecuredShareMemory::TakeAccess() { return SECMEM_TakeAccess(m_stSecMem); }

int CSecuredShareMemory::ReleaseAccess() { return SECMEM_ReleaseAccess(m_stSecMem); }

void *CSecuredShareMemory::GetBaseAddress() { return SECMEM_GetBaseAddress(m_stSecMem); }

int CSecuredShareMemory::Read(char *buff, size_t size) { return SECMEM_Read(m_stSecMem, buff, size); }

int CSecuredShareMemory::Write(char *buff, size_t size) { return SECMEM_Write(m_stSecMem, buff, size); }

int64_t CSecuredShareMemory::Seek(int64_t pos, uint32_t type) { return SECMEM_Seek(m_stSecMem, pos, type); }

int64_t CSecuredShareMemory::CurrentPos() { return Seek(0, 1); }

} // namespace osac
} // namespace gbs