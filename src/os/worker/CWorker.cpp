#include "CWorker.h"
#include "dbg/Debug.h"

#define SafeWriteWorkerState(state) MtxSafeWriteInt32(m_stMtx, m_s32WorkerState, state)
#define SafeReadWorkerState()       m_stMtx.SafeRead(m_s32WorkerState)

namespace gbs
{
    namespace osac
    {
        CWorker::CWorker(const char *cWorkName, IWorker *iWorker, int priority, int core) :
            m_strName(std_str(cWorkName)),
            m_pcIWorker(iWorker),
            m_s32WorkerState(WORKER_STOP),
            m_pParamBuff(NULL),
            m_sParamLen(0),
            m_workFunc(NULL) {

            m_pcThread = new CThread();
            if (m_pcThread->Create(cWorkName, std::bind(&CWorker::WorkerRunning, this, std::placeholders::_1),
                                   NULL, priority, core) < 0) {
                CLOG_ERROR("Create worker thread failed\n");
            }

            if (m_stMtx.Create() < 0) {
                CLOG_ERROR("Create worker mutex failed\n");
            }
        }

        CWorker::CWorker(const char *cWorkName, WorkFunction fnc) :
            m_strName(std_str(cWorkName)),
            m_pcIWorker(NULL),
            m_s32WorkerState(WORKER_STOP),
            m_pParamBuff(NULL),
            m_sParamLen(0),
            m_workFunc(fnc) {

            m_pcThread = new CThread();
            if (m_pcThread->Create(cWorkName, std::bind(&CWorker::WorkerRunning, this, std::placeholders::_1),
                                   NULL, -1, -1) < 0) {
                CLOG_ERROR("Create worker thread failed\n");
            }

            if (m_stMtx.Create() < 0) {
                CLOG_ERROR("Create worker mutex failed\n");
            }
        }
        CWorker::~CWorker() {
            if (TerminateWorker() < 0) {
                CLOG_ERROR("Terminate worker timeout\n");
            }

            m_stMtx.Destroy();
            if (m_pParamBuff) {
                delete[] m_pParamBuff;
                m_pParamBuff = NULL;
            }
            delete m_pcThread;
            m_pcThread = NULL;
        }

        void *CWorker::WorkerRunning(void *param) try {

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
                if (m_pcIWorker)
                    m_pcIWorker->OnWorkerRun(m_pParamBuff, m_sParamLen);
                if (m_workFunc)
                    m_workFunc(m_pParamBuff);
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
        } catch (std::exception &ex) {
            return NULL;
        }

        int CWorker::StartWorker(const char *param, size_t len) {
            if (m_pParamBuff) {
                delete[] m_pParamBuff;
                m_pParamBuff = NULL;
            }
            if (param && len > 0) {
                m_pParamBuff = new char[len];
                memcpy(m_pParamBuff, param, len);
                m_sParamLen = len;
            }

            try {
                if (m_pcIWorker)
                    m_pcIWorker->OnRequestWorkerStart();
                SafeWriteWorkerState(WORKER_INIT);
            } catch (std::exception &exp) {}
            return m_pcThread->Start();
        }

        int CWorker::StopWorker() {
            try {
                if (m_pcIWorker)
                    m_pcIWorker->OnRequestWorkerStop();
                SafeWriteWorkerState(WORKER_FINAL);
                while (SafeReadWorkerState() != static_cast<int32_t>(WORKER_STOP))
                    ;
            } catch (std::exception &exp) {}
            return RET_OK;
        }

        int CWorker::TerminateWorker(uint64_t ms) {
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

        int CWorker::JoinWorker() {
            return m_pcThread->Join();
        }

        int CWorker::DetachWorker() {
            return m_pcThread->Detach();
        }
    } // namespace osac
} // namespace gbs