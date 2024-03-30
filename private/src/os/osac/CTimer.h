#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "osal/OSAL.h"

namespace gbs {
namespace osac {
class __DLL_DECLSPEC__ CTimer {
private:
    TIMER_T m_stTimer;
    TIMER_Callback m_pFnc;
    void *m_pParam;
    static void TimerCallback(void *);

public:
    CTimer(int interval, TIMER_Callback fnc = NULL, void *param = NULL, int start = 1);
    virtual ~CTimer();

    static int Initialize();
    int Start();
    int Stop();
    int SetInterval(int interval);

    virtual void TimerElapsed(void *param);
};
}; // namespace osac
} // namespace gbs
#endif // __CTIMER_H__