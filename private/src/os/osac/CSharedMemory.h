/**
 * @file CSharedMemory.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __CSHAREDMEMORY_H__
#define __CSHAREDMEMORY_H__

#include "common/Typedef.h"
#include "osal/OSAL.h"

namespace gbs {
namespace osac {
class __DLL_DECLSPEC__ CSharedMemory {
private:
    SHM_t m_stShm;

public:
    CSharedMemory() {}
    ~CSharedMemory() {}

    int Open(const char *name, size_t size);
    int Create(const char *name, size_t size);
    const SHM_t *GetSHM() const { return &m_stShm; }
    int Close();
    int Release();

    int Read(char *buff, size_t size);
    int Write(char *buff, size_t size);
    int Seek(int64_t pos, uint32_t type = 0);
    size_t Size() const { return m_stShm.size; }
};
}; // namespace osac
} // namespace gbs
#endif // __CSHAREDMEMORY_H__