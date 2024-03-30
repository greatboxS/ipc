#ifndef __CLOCKGUARD_H__
#define __CLOCKGUARD_H__

#include "common/Typedef.h"
#include "CMutex.h"

#define __lock_guard(mtx)                  osac::CLockGuard __lockguard_unique__(mtx);
#define __lock_guard_timeout(mtx, timeout) osac::CLockGuard __lockguard_unique__(mtx, timeout);

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CLockGuard
        {
        private:
            CMutex &m_mtx;
            CLockGuard() = delete;
            CLockGuard(CLockGuard &) = delete;

        public:
            explicit CLockGuard(CMutex &mtx, int timeout = 0) :
                m_mtx(mtx) {
                m_mtx.Lock(timeout);
            }

            ~CLockGuard() {
                m_mtx.UnLock();
            }
        };
    }; // namespace osac
} // namespace gbs

#endif // __CLOCKGUARD_H__