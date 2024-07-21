#include "osac.h"
#include "cthread.h"
#include "osal/ipc_mutex.h"
#include "osal/ipc_semaphore.h"
#include "osal/ipc_thread.h"
#include <string.h>
#include <chrono>

namespace ipc::core {
cthread::cthread() :
    m_s32Terminated(0), m_s32State(eTHREAD_UNKNOWN),
    m_valid(std::make_shared<int>()),
    m_handle({this, m_valid}) {
    memset(&m_stThreadInfo, 0, sizeof(m_stThreadInfo));
}

cthread::~cthread() {
    stop();
    finalize();
    wait_for_complete();
    if (m_s32Terminated) {
        return;
    }
    terminate();
}

/**
 * @fn handle
 * @brief ThreadHandler
 *
 * @param param
 * @return TASK_RET
 */
TASK_RET cthread::handle(VOID_T param) try {
    if (!param) {
        return nullptr;
    }

    Handle_t handle = *reinterpret_cast<Handle_t *>(param);
    int32_t s32State = 0;

    if (mutex_safe_read(handle.t->m_s32State, handle.t->m_stMutex) == static_cast<int32_t>(eTHREAD_CREATED)) {
        mutex_safe_write(handle.t->m_s32State, handle.t->m_stMutex, static_cast<int32_t>(eTHREAD_INITED));
        handle.t->on_thread_initialize();
    }

    while (!handle.valid.expired()) {
        s32State = mutex_safe_read(handle.t->m_s32State, handle.t->m_stMutex);
        if (s32State == static_cast<int32_t>(eTHREAD_RUNNING)) {
            handle.t->wrap();
        } else if (s32State == static_cast<int32_t>(eTHREAD_FINALIZED)) {
            break;
        } else {
            cthread::delay(1);
        }
    }

    handle.t->on_thread_finalize();
    mutex_safe_write(handle.t->m_s32State, handle.t->m_stMutex, static_cast<int32_t>(eTHREAD_TERMINATED));
    return nullptr;
} catch (...) {
    // Do nothing
    return nullptr;
}

/**
 * @fn create
 * @brief create new thread
 *
 * @param name 		Thread name
 * @param callback 	Callback function, set it in the case that run is not overide
 * @param param 	Callback parameter
 * @param priority 	Thread priority
 * @param core 		Thread stick to core
 * @return int 		0 if success, otherwise -1
 */
int cthread::create(const char *name, CThreadCallbackFunction callback, void *param, int priority, int core) {
    if (mutex_create(m_stMutex) != 0) {
        return -1;
    }

    m_s32State = eTHREAD_CREATED;
    if (thread_create(m_stThreadInfo, name, cthread::handle, &m_handle, priority, core) < 0) {
        OSAC_ERR("create new thread %s failed (%d)", name, __ERROR__);
        return -1;
    }
    m_pCallback = callback;
    m_pParam = param;
    return 0;
}

/**
 * @fn join
 * @brief join current thread
 *
 * @return int
 */
int cthread::join() { return thread_join(m_stThreadInfo); }

/**
 * @fn detach
 * @brief detach current thread
 *
 * @return int
 */
int cthread::detach() { return thread_detach(m_stThreadInfo); }

/**
 * @fn start
 * @brief start current thread
 *
 * @return int
 */
int cthread::start() {
    if (mutex_safe_read(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_RUNNING)) {
        mutex_safe_write(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_RUNNING));
    }
    return thread_run(m_stThreadInfo);
}

/**
 * @fn stop
 * @brief stop current thread
 *
 * @return int
 */
int cthread::stop() {
    if (mutex_safe_read(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_STOPPING)) {
        mutex_safe_write(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_STOPPING));
    }
    return 0;
}

/**
 * @fn finalize
 * @brief finalize current thread
 *
 * @return int
 */
int cthread::finalize() {
    if (mutex_safe_read(m_s32State, m_stMutex) <= static_cast<int32_t>(eTHREAD_FINALIZED)) {
        mutex_safe_write(m_s32State, m_stMutex, static_cast<int32_t>(eTHREAD_FINALIZED));
    }
    return 0;
}

bool cthread::wait_for_complete(uint64_t ms) {
    auto stime = std::chrono::high_resolution_clock::now();
    uint64_t tick = 0;
    while (mutex_safe_read(m_s32State, m_stMutex) != static_cast<int32_t>(eTHREAD_TERMINATED)) {
        cthread::delay(1);
        if (++tick >= ms) {
            break;
        }
    }
    mutex_destroy(m_stMutex);
    return (mutex_safe_read(m_s32State, m_stMutex) == static_cast<int32_t>(eTHREAD_TERMINATED));
}

/**
 * @fn terminate
 * @brief RequestFinalize
 *
 * @return int
 */
int cthread::terminate() {
    m_s32Terminated = 1;
    thread_terminate(m_stThreadInfo);
    m_s32State = static_cast<int32_t>(eTHREAD_TERMINATED);
    return 0;
}

/**
 * @fn run
 * @brief Thread run function, user have to overwrite this function
 * 		  if callback in thread_create function is not set
 *
 * @return void*
 */
void *cthread::run() {
    // thread_delay(1);
    return nullptr;
}

/**
 * @fn delay
 * @brief
 *
 */
void cthread::delay(unsigned int ms, unsigned int us) {
    thread_delay(ms, us);
}
} // namespace ipc::core