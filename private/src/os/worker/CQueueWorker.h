#ifndef __CQUEUEWORKER_H__
#define __CQUEUEWORKER_H__

#include "Worker.h"
#include "osac/CThread.h"
#include "osac/CMutex.h"
#include <string>
#include <queue>

namespace gbs {
namespace osac {
struct WorkerItem_t {
    void *pBuff;
    uint32_t u32BuffSize;
    WorkFunction fnc;
};

class __DLL_DECLSPEC__ CQueueWorker {
private:
    IWorker *m_pcIWorker;
    CThread *m_pcThread;
    CMutex m_stMtx;
    std::string m_strName;
    int32_t m_s32WorkerState;
    std::queue<WorkerItem_t *> m_qWorkerQueue;

    void *WorkerRunning(void *param);
    void EnQueueWorker(WorkerItem_t *wk);
    WorkerItem_t *DeQueueWorker();
    void DeleteWorkerItem(WorkerItem_t *wk);

public:
    explicit CQueueWorker(const char *cWorkName, IWorker *iWorker = NULL, int priority = 0, int core = -1);
    virtual ~CQueueWorker();

    virtual int IsRunning() { return (m_stMtx.SafeRead(m_s32WorkerState) == WORKER_RUN); }
    virtual int StartWorker();
    virtual int StopWorker();
    virtual int TerminateWorker(uint64_t ms = DEFAULT_WORKER_TERMINATION_TIMEOUT);
    virtual int DoWork(const char *param, uint32_t size);
    virtual int DoWork(WorkFunction fnc, void *param);
    virtual int JoinWorker();
    virtual int DetachWorker();
};
} // namespace osac
} // namespace gbs

#endif // __CQUEUEWORKER_H__