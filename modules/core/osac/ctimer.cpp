#include "osal/timer.h"
#include "ctimer.h"
#include "functional"

namespace ipc::core {
ctimer::ctimer(int interval, TIMER_Callback fnc, void *param, int start) {
    m_stTimer = timer_create(interval, ctimer::timer_callback, this, NULL, start);
    m_pFnc = fnc;
    m_pParam = param;
}

void ctimer::timer_callback(void *param) {
    ctimer *timer = static_cast<ctimer *>(param);
    if (timer->m_pFnc) timer->m_pFnc(timer->m_pParam);
    timer->timer_elapsed(timer->m_stTimer.param);
}

ctimer::~ctimer() { timer_terminate(m_stTimer); }

int ctimer::initialize() { return timer_initialize(); }

int ctimer::start() { return timer_start(m_stTimer); }

int ctimer::stop() { return timer_stop(m_stTimer); }

int ctimer::SetInterval(int interval) { return timer_set_initerval(m_stTimer, interval); }

void ctimer::timer_elapsed(void *param) {
}
} // namespace ipc::core
