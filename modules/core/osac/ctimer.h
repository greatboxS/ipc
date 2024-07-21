#ifndef __CTIMER_H__
#define __CTIMER_H__

#include "osal/osal.h"

namespace ipc::core {
class __dll_declspec__ ctimer {
private:
    TIMER_T m_stTimer;
    TIMER_Callback m_pFnc;
    void *m_pParam;
    static void timer_callback(void *);

public:
    ctimer(int interval, TIMER_Callback fnc = NULL, void *param = NULL, int start = 1);
    virtual ~ctimer();

    static int initialize();
    int start();
    int stop();
    int SetInterval(int interval);

    virtual void timer_elapsed(void *param);
};
} // namespace ipc::core
#endif // __CTIMER_H__