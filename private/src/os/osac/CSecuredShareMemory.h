#ifndef __CSECUREDSHAREMEMORY_H__
#define __CSECUREDSHAREMEMORY_H__

#include "osal/OSAL.h"

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CSecuredShareMemory
        {
        private:
            SECURED_SHARE_MEMORY_T m_stSecMem;
            char m_strName[SHM_NAME_SIZE];
            size_t m_size;

        public:
            CSecuredShareMemory(const char *name, size_t size);
            ~CSecuredShareMemory();

            int Create();
            int Destroy();
            int Open();
            int Close();
            int IsOpen();

            int TakeAccess();
            int ReleaseAccess();
            void *GetBaseAddress();

            int Read(char *buff, size_t size);
            int Write(char *buff, size_t size);
            size_t Size() const { return m_stSecMem.shm.size; }

            /**
             * @fn Seek
             * @brief
             *
             * @param pos
             * @param type 0 begin, 1 current, 2 end
             * @return int64
             */
            int64_t Seek(int64_t pos, uint32_t type);
            int64_t CurrentPos();
        };

        template <typename T>
        class CSharedMemInstance
        {
            CSecuredShareMemory *m_sharedMem;

        public:
            CSharedMemInstance(const char *name) {
                m_sharedMem = new CSecuredShareMemory(name, sizeof(T));
            }
            ~CSharedMemInstance() {
                delete m_sharedMem;
            }

            int Create() { return m_sharedMem->Create(); }
            int Destroy() { return m_sharedMem->Destroy(); }
            int TakeAccess() { return m_sharedMem->TakeAccess(); }
            int ReleaseAccess() { return m_sharedMem->ReleaseAccess(); }
            size_t Size() { return m_sharedMem->Size(); }
            T *Get() { return reinterpret_cast<T *>(m_sharedMem->GetBaseAddress()); }
        };
    }; // namespace osac
} // namespace gbs

#endif // __CSECUREDSHAREMEMORY_H__