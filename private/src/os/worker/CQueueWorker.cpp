#include "CQueueWorker.h"
#include "dbg/Debug.h"
#include "osac/CLockGuard.h"

#define SafeWriteWorkerState(state) MtxSafeWriteInt32(m_stMtx, m_s32WorkerState, state)
#define SafeReadWorkerState()       m_stMtx.SafeRead(m_s32WorkerState)

namespace gbs {
namespace osac {
CQueueWorker::CQueueWorker(const char *cWorkName, IWorker *iWorker, int priority, int core) :
    m_strName(std_str(cWorkName)),
    m_pcIWorker(iWorker),
    m_s32WorkerState(WORKER_STOP) {

    m_pcThread = new CThread();
    if (m_pcThread->Create(cWorkName, std::bind(&CQueueWorker::WorkerRunning, this, std::placeholders::_1),
                           NULL, priority, core)
        < 0) {
        CLOG_ERROR("Create worker thread failed\n");
    }

    if (m_stMtx.Create() < 0) {
        CLOG_ERROR("Create worker mutex failed\n");
    }
}

CQueueWorker::~CQueueWorker() {
    if (TerminateWorker() < 0) {
        CLOG_ERROR("Terminate worker timeout\n");
    }
    m_stMtx.Destroy();
    delete m_pcThread;
    while (true) {
        auto wk = DeQueueWorker();
        if (!wk) break;
        DeleteWorkerItem(wk);
    }
}

void *CQueueWorker::WorkerRunning(void *param) try {

    WorkerItem_t *pWkItem = NULL;
    int32_t state = SafeReadWorkerState();
    switch (state) {

    case WORKER_INIT:
        SafeWriteWorkerState(WORKER_RUN);
        if (!m_pcIWorker) break;
        if (m_pcIWorker->OnWorkerInitialize() < 0) {
            CLOG_WARN("Worker initialize failed\n");
        }
        break;

    case WORKER_FINAL:
        SafeWriteWorkerState(WORKER_STOP);
        if (!m_pcIWorker) break;
        if (m_pcIWorker->OnWorkerFinalize() < 0) {
            CLOG_WARN("Worker finalize failed\n");
        }
        break;

    case WORKER_RUN:
        pWkItem = DeQueueWorker();
        if (!pWkItem) break;
        /* IWorker class handler function */
        if (!m_pcIWorker) break;

        m_pcIWorker->OnWorkerRun(pWkItem->pBuff, pWkItem->u32BuffSize);
        if (pWkItem->pBuff)
            delete[](char *) pWkItem->pBuff;
        /* Single function handler */
        if (pWkItem->fnc) {
            pWkItem->fnc(pWkItem->pBuff);
        }
        DeleteWorkerItem(pWkItem);
        break;

    case WORKER_PRE_EXIT:
        SafeWriteWorkerState(WORKER_EXIT_DONE);
        break;

    case WORKER_EXIT_DONE:
    case WORKER_STOP:
    default:
        CThread::Delay(10);
        break;
    }
    return NULL;
} catch (std::exception &ex) { return NULL; }

void CQueueWorker::EnQueueWorker(WorkerItem_t *wk) {
    __lock_guard(m_stMtx);
    m_qWorkerQueue.push(wk);
}

WorkerItem_t *CQueueWorker::DeQueueWorker() {
    WorkerItem_t *wk = NULL;
    __lock_guard(m_stMtx);
    if (m_qWorkerQueue.size() == 0) return wk;

    wk = m_qWorkerQueue.front();
    m_qWorkerQueue.pop();
    return wk;
}

void CQueueWorker::DeleteWorkerItem(WorkerItem_t *wk) {
    if (!wk) return;
    delete wk;
}

int CQueueWorker::StartWorker() {
    try {
        if (m_pcIWorker)
            m_pcIWorker->OnRequestWorkerStart();
        SafeWriteWorkerState(WORKER_INIT);
    } catch (std::exception &ex) {}
    return m_pcThread->Start();
}

int CQueueWorker::StopWorker() {
    try {
        if (m_pcIWorker)
            m_pcIWorker->OnRequestWorkerStop();
        SafeWriteWorkerState(WORKER_FINAL);
        while (SafeReadWorkerState() != static_cast<int32_t>(WORKER_STOP))
            ;
    } catch (std::exception &ex) {}
    return RET_OK;
}

int CQueueWorker::TerminateWorker(uint64_t ms) {
    uint64_t tick = 0;
    if (SafeReadWorkerState() == WORKER_EXIT_DONE) return RET_OK;
    SafeWriteWorkerState(WORKER_PRE_EXIT);
    while (SafeReadWorkerState() != static_cast<int32_t>(WORKER_EXIT_DONE)) {
        CThread::Delay(1);
        tick++;
        if (tick > ms) break;
    };

    if (SafeReadWorkerState() == static_cast<int32_t>(WORKER_EXIT_DONE))
        return RET_OK;
    else
        return RET_ERR;
}

int CQueueWorker::DoWork(const char *param, uint32_t len) {
    WorkerItem_t *wk = new WorkerItem_t;
    wk->pBuff = NULL;
    wk->u32BuffSize = 0;

    if (param && len > 0) {
        wk->pBuff = new char[len];
        wk->u32BuffSize = len;
        memcpy(wk->pBuff, param, len);
    }
    EnQueueWorker(wk);
    return RET_OK;
}

int CQueueWorker::DoWork(WorkFunction fnc, void *param) {
    WorkerItem_t *wk = new WorkerItem_t;
    wk->pBuff = param;
    wk->u32BuffSize = 0;
    wk->fnc = fnc;
    EnQueueWorker(wk);
    return RET_OK;
}

int CQueueWorker::JoinWorker() {
    return m_pcThread->Join();
}

int CQueueWorker::DetachWorker() {
    return m_pcThread->Detach();
}
} // namespace osac
} // namespace gbs