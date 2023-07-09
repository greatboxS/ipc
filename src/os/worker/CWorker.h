#ifndef __CWORKER_H__
#define __CWORKER_H__

#include "Worker.h"
#include "osac/CMutex.h"
#include "osac/CThread.h"
#include <string>

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CWorker
        {
        private:
            IWorker *m_pcIWorker;
            CThread *m_pcThread;
            CMutex m_stMtx;
            std::string m_strName;
            int32_t m_s32WorkerState;
            char *m_pParamBuff;
            size_t m_sParamLen;
            WorkFunction m_workFunc;

            void *WorkerRunning(void *param);

        public:
            explicit CWorker(const char *cWorkName, IWorker *iWorker, int priority = 0, int core = -1);
            explicit CWorker(const char *cWorkName, WorkFunction fnc);
            virtual ~CWorker();

            virtual int IsRunning() { return (m_stMtx.SafeRead(m_s32WorkerState) == WORKER_RUN); }
            virtual int StartWorker(const char *param = NULL, size_t len = 0);
            virtual int StopWorker();
            virtual int TerminateWorker(uint64_t ms = DEFAULT_WORKER_TERMINATION_TIMEOUT);
            virtual int JoinWorker();
            virtual int DetachWorker();
        };
    } // namespace osac
} // namespace gbs
#endif // __CWORKER_H__