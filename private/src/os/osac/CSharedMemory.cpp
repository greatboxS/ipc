#include "CSharedMemory.h"
#include "osal/SharedMemory.h"

namespace gbs
{
    namespace osac
    {
        int CSharedMemory::Open(const char *name, size_t size) { return osal::SHM_Open(m_stShm, name, size); }

        int CSharedMemory::Create(const char *name, size_t size) { return osal::SHM_Create(m_stShm, name, size); }

        int CSharedMemory::Close() { return osal::SHM_Close(m_stShm); }

        int CSharedMemory::Release() { return osal::SHM_Destroy(m_stShm); }

        int CSharedMemory::Read(char *buff, size_t size) { return osal::SHM_Read(m_stShm, buff, size); }

        int CSharedMemory::Write(char *buff, size_t size) { return osal::SHM_Write(m_stShm, buff, size); }

        int CSharedMemory::Seek(int64_t pos, uint32_t type) { return osal::SHM_Seek(m_stShm, pos, type); }
    } // namespace osac
} // namespace gbs
