#include "timer.h"
#include "ipc_mutex.h"
#include "ipc_thread.h"

#include <chrono>
#include <string.h>
#include <string>
#include <vector>

namespace ipc::core {
#define TIMER_TICK        1000
#define clock_now()       std::chrono::high_resolution_clock::now()
#define clock_duration(x) std::chrono::duration_cast<std::chrono::microseconds>(x)

struct timer_wrap_t {
    TIMER_T timer;
    int tick;
    int start;
};

struct timer_manager_t {
    MUTEX_t mtx;
    std::vector<timer_wrap_t> timers;
};

/* TIMER MANAGER OBJECTS */
static THREAD_T base_timer_thread;
static timer_manager_t timer_manager;
static int timer_initialized = 0;
static int timer_index = 0;
static auto timer_begin_time = clock_now();

/**
 * @fn timer_run
 * @brief Timer thread loop base
 *
 * @param param
 * @return void*
 */
static TASK_RET timer_run(TASK_ARG param) {
    while (true) {

#if defined(WIN32) || defined(_WIN32)
        while (clock_duration(clock_now() - timer_begin_time).count() < TIMER_TICK) {}
        timer_begin_time = clock_now();
#else
        ipc::core::thread_delay(1);
#endif

        mutex_lock(timer_manager.mtx);
        for (timer_wrap_t &wrapper : timer_manager.timers) {
            if (!wrapper.start) {
                continue;
            }
            wrapper.tick++;
            if (wrapper.tick >= wrapper.timer.interval) {
                wrapper.tick = 0;
                if (wrapper.timer.fnc) {
                    wrapper.timer.fnc(wrapper.timer.param);
                }
            }
        }
        mutex_unlock(timer_manager.mtx);
    }
    return nullptr;
}

/**
 * @fn timer_initialize
 * @brief Initialize timer loop
 *
 * @return int
 */
int timer_initialize() {
    if (mutex_create(timer_manager.mtx) < 0) {
        return RET_ERR;
    }

    if (thread_create(base_timer_thread, "__timer_thread", timer_run, &timer_manager, 0) != 0) {
        return RET_ERR;
    }
    thread_run(base_timer_thread);
    thread_detach(base_timer_thread);
    return RET_OK;
}

/**
 * @fn timer_create
 * @brief Create Timer using thread base
 *
 * @param interval  Timer interval in ms
 * @param fnc       Callback when timeout
 * @param name      Timer name
 * @param start     Start after create
 * @return TIMER_T
 */
TIMER_T timer_create(int interval, TIMER_Callback fnc, void *param, const char *name, int start) {
    timer_wrap_t wrapper;

    memset(&wrapper, 0, sizeof(wrapper));

    if (timer_initialized == 0) {
        if (timer_initialize() < 0) {
            return wrapper.timer;
        }
        else {
            timer_initialized = 1;
        }
    }

    if (interval <= 0 || !fnc) {
        return wrapper.timer;
    }

    wrapper.tick = 0;
    wrapper.start = start;
    wrapper.timer.fnc = fnc;
    wrapper.timer.interval = interval;
    wrapper.timer.param = param;
    wrapper.timer.id = ++timer_index;

    if (name) {
        strncpy(wrapper.timer.name, name, sizeof(wrapper.timer.name));
    }

    mutex_lock(timer_manager.mtx);
    timer_manager.timers.push_back(wrapper);
    mutex_unlock(timer_manager.mtx);

    return wrapper.timer;
}

/**
 * @fn timer_start
 * @brief
 *
 * @param timer
 * @return int
 */
int timer_start(TIMER_T &timer) {
    int ret = -1;
    mutex_lock(timer_manager.mtx);
    for (auto &wrapper : timer_manager.timers) {
        if (timer.id == wrapper.timer.id) {
            wrapper.start = 1;
            ret = 0;
            break;
        }
    }
    mutex_unlock(timer_manager.mtx);
    return ret;
}

/**
 * @fn timer_stop
 * @brief
 *
 * @param timer
 * @return int
 */
int timer_stop(TIMER_T &timer) {
    int ret = -1;
    mutex_lock(timer_manager.mtx);
    for (auto &wrapper : timer_manager.timers) {
        if (timer.id == wrapper.timer.id) {
            wrapper.start = 0;
            ret = 0;
            break;
        }
    }
    mutex_unlock(timer_manager.mtx);
    return ret;
}

/**
 * @fn timer_set_initerval
 * @brief
 *
 * @param timer
 * @param interval
 * @return int
 */
int timer_set_initerval(TIMER_T &timer, int interval) {
    int ret = -1;
    mutex_lock(timer_manager.mtx);
    for (auto &wrapper : timer_manager.timers) {
        if (timer.id == wrapper.timer.id) {
            wrapper.timer.interval = interval;
            break;
        }
    }
    mutex_unlock(timer_manager.mtx);
    return ret;
}

/**
 * @fn timer_terminate
 * @brief
 *
 * @param timer
 * @return int
 */
int timer_terminate(TIMER_T &timer) {
    int ret = -1;
    mutex_lock(timer_manager.mtx);
    for (auto i = timer_manager.timers.begin(); i != timer_manager.timers.end(); i++) {
        if (timer.id == i->timer.id) {
            timer_manager.timers.erase(i);
            break;
        }
    }
    mutex_unlock(timer_manager.mtx);
    return ret;
}
} // namespace ipc::core