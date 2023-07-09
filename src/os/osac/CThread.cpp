#include "CThread.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include "osal/Semaphore.h"
#include "osal/Thread.h"
#include <string.h>
#include <chrono>

namespace gbs
{
    namespace osac
    {
        CThread::CThread() :
            m_s32Terminated(0), m_s32State(eTHREAD_UNKNOWN),
            m_valid(std::make_shared<int>()),
            m_handle({this, m_valid}) {
            memset(&m_stThreadInfo, 0, sizeof(m_stThreadInfo));
        }

        CThread::~CThread() {
            Stop();
            Finalize();
            WaitForCompletion();
            if (m_s32Terminated) return;
            Terminate();
        }

        /**
         * @fn Handle
         * @brief ThreadHandler
         *
         * @param param
         * @return TASK_RET
         */
        TASK_RET CThread::Handle(VOID_T param) try {
            if (!param) return NULL;

            Handle_t handle = *reinterpret_cast<Handle_t *>(param);
            int32_t s32State = 0;

            if (osal::MUTEX_SafeRead(handle.t->m_s32State, handle.t->m_stMutex) == static_cast<int32_t>(eTHREAD_CREATED)) {
                osal::MUTEX_SafeWrite(handle.t->m_s32State, handle.t->m_stMutex, static_cast<int32_t>(eTHREAD_INITED));
                handle.t->OnThreadInitialize();
            }

            while (!handle.valid.expired()) {
                s32State = osal::MUTEX_SafeRead(handle.t->m_s32State, handle.t->m_stMutex);
                if (s32State == static_cast<int32_t>(eTHREAD_RUNNING)) {
                    handle.t->Wrapper();
                } else if (s32State == static_cast<int32_t>(eTHREAD_FINALIZED)) {
                    break;
                } else {
                    CThread::Delay(1);
                }
            }

            handle.t->OnThreadFinalize();
            osal::MUTEX_SafeWrite(handle.t->m_s32State, handle.t->m_stMutex, static_cast<int32_t>(eTHREAD_TERMINATED));
            return NULL;
        }
        _EXCEPT_CATCH(return NULL)

        /**
         * @fn Create
         * @brief Create new thread
         *
         * @param name 		Thread name
         * @param callback 	Callback function, set it in the case that Run is not overide
         * @param param 	Callback parameter
         * @param priority 	Thread priority
         * @param core 		Thread stick to core
         * @return int 		0 if success, otherwise -1
         */
        int CThread::Create(const char *name, CThreadCallbackFunction callback, void *param, int priority, int core) {
            if (osal::MUTEX_Create(m_stMutex) != 0) return -1;

            m_s32State = eTHREAD_CREATED;
            if (osal::THREAD_Create(m_stThreadInfo, name, CThread::Handle, &m_handle, priority, core) < 0) {
                CLOG_ERROR("Create new thread %s failed (%d)", name, __ERROR__);
                return -1;
            }
            m_pCallback = callback;
            m_pParam = param;
            return 0;
        }

        /**
         * @fn Join
         * @brief Join current thread
         *
         * @return int
         */
        int CThread::Join() { return osal::THREAD_Join(m_stThreadInfo); }

        /**
         * @fn Detach
         * @brief Detach current thread
         *
         * @return int
         */
        int CThread::Detach() { return osal::THREAD_Detach(m_stThreadInfo); }

        /**
         * @fn Start
         * @brief Start current thread
         *
         * @return int
         */
        int CThread::Start() {
            if (osal::MUTEX_SafeRead(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_RUNNING))
                osal::MUTEX_SafeWrite(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_RUNNING));
            return osal::THREAD_Run(m_stThreadInfo);
        }

        /**
         * @fn Stop
         * @brief Stop current thread
         *
         * @return int
         */
        int CThread::Stop() {
            if (osal::MUTEX_SafeRead(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_STOPPING))
                osal::MUTEX_SafeWrite(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_STOPPING));
            return 0;
        }

        /**
         * @fn Finalize
         * @brief Finalize current thread
         *
         * @return int
         */
        int CThread::Finalize() {
            if (osal::MUTEX_SafeRead(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_FINALIZED))
                osal::MUTEX_SafeWrite(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_FINALIZED));
            return 0;
        }

        bool CThread::WaitForCompletion(uint64_t ms) {
            auto stime = std::chrono::high_resolution_clock::now();
            uint64_t tick = 0;
            while (osal::MUTEX_SafeRead(m_s32State, m_stMutex) != static_cast<int32_t>(eTHREAD_TERMINATED)) {
                CThread::Delay(1);
                if (++tick >= ms) break;
            }
            osal::MUTEX_Destroy(m_stMutex);
            return (osal::MUTEX_SafeRead(m_s32State, m_stMutex) == static_cast<int32_t>(eTHREAD_TERMINATED));
        }

        /**
         * @fn Terminate
         * @brief RequestFinalize
         *
         * @return int
         */
        int CThread::Terminate() {
            m_s32Terminated = 1;
            osal::THREAD_Terminate(m_stThreadInfo);
            m_s32State = static_cast<int32_t>(eTHREAD_TERMINATED);
            return 0;
        }

        /**
         * @fn Run
         * @brief Thread Run function, user have to overwrite this function
         * 		  if callback in THREAD_Create function is not set
         *
         * @return void*
         */
        void *CThread::Run() {
            // THREAD_Delay(1);
            return NULL;
        }

        /**
         * @fn Delay
         * @brief
         *
         */
        void CThread::Delay(unsigned int ms, unsigned int us) {
            osal::THREAD_Delay(ms, us);
        }
    } // namespace osac
} // namespace gbs
