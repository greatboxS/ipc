#include "CTimer.h"
#include "dbg/Debug.h"
#include "functional"
#include "osal/Timer.h"

namespace gbs
{
    namespace osac
    {
        CTimer::CTimer(int interval, TIMER_Callback fnc, void *param, int start) {
            m_stTimer = osal::TIMER_Create(interval, CTimer::TimerCallback, this, NULL, start);
            m_pFnc = fnc;
            m_pParam = param;
        }

        void CTimer::TimerCallback(void *param) {
            CTimer *timer = static_cast<CTimer *>(param);
            if (timer->m_pFnc) timer->m_pFnc(timer->m_pParam);
            timer->TimerElapsed(timer->m_stTimer.param);
        }

        CTimer::~CTimer() { osal::TIMER_Terminate(m_stTimer); }

        int CTimer::Initialize() { return osal::TIMER_Initialize(); }

        int CTimer::Start() { return osal::TIMER_Start(m_stTimer); }

        int CTimer::Stop() { return osal::TIMER_Stop(m_stTimer); }

        int CTimer::SetInterval(int interval) { return osal::TIMER_SetInterval(m_stTimer, interval); }

        void CTimer::TimerElapsed(void *param) { /* CLOG_INFO("Timer %d elapsed\n", mTimer.id); */
        }
    }; // namespace osac
} // namespace gbs
