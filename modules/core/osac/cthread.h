/**
 * @file cthread.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef CTHREAD_H
#define CTHREAD_H

#include "osal/osal.h"
#include <functional>
#include <memory>

#define DEFAULT_WAIT_TIME 10000U

namespace ipc::core {
using CThreadCallbackFunction = std::function<void *(void *)>;

class __dll_declspec__ cthread {
    enum eCThreadState {
        eTHREAD_UNKNOWN = -1,
        eTHREAD_CREATED,
        eTHREAD_INITED,
        eTHREAD_RUNNING,
        eTHREAD_STOPPING,
        eTHREAD_FINALIZED,
        eTHREAD_TERMINATED,
    };

    struct Handle_t {
        cthread *t;
        std::weak_ptr<int> valid;
    };

private:
    THREAD_T m_stThreadInfo;
    MUTEX_T m_stMutex;
    int32_t m_s32State;
    int32_t m_s32Terminated;
    void *m_pParam;
    CThreadCallbackFunction m_pCallback;
    std::shared_ptr<int> m_valid;
    Handle_t m_handle;

    /**
     * @fn handle
     * @brief ThreadHandler
     *
     * @param param
     * @return TASK_RET
     */
    static TASK_RET handle(VOID_T param);
    void wrap() {
        run();
        if (m_pCallback) {
            m_pCallback(m_pParam);
        }
    }

public:
    cthread();
    virtual ~cthread();

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
    int create(const char *name, CThreadCallbackFunction callback = NULL, void *param = NULL, int priority = 0, int core = -1);

    /**
     * @fn join
     * @brief join current thread
     *
     * @return int
     */
    int join();

    /**
     * @fn detach
     * @brief detach current thread
     *
     * @return int
     */
    int detach();

    /**
     * @fn start
     * @brief start current thread
     *
     * @return int
     */
    int start();

    /**
     * @fn stop
     * @brief stop current thread
     *
     * @return int
     */
    int stop();

    /**
     * @fn finalize
     * @brief finalize current thread
     *
     * @return int
     */
    int finalize();

    /**
     * @fn wait_for_complete
     * @brief Wait for thread to be completed after finalize is called
     *
     * @param ms
     * @return bool true if completion, otherwise false
     */
    bool wait_for_complete(uint64_t ms = DEFAULT_WAIT_TIME);

    /**
     * @fn terminate
     * @brief RequestFinalize
     *
     * @return int
     */
    int terminate();

    /**
     * @fn on_thread_initialize
     * @brief Onetime calling function, used to the initialization
     *
     * @return int
     */
    virtual int on_thread_initialize() { return 0; }

    /**
     * @fn on_thread_finalize
     * @brief On thread finalize
     *
     * @return int
     */
    virtual int on_thread_finalize() { return 0; }

    /**
     * @fn run
     * @brief Thread run function, user have to overwrite this function
     * 		  if callback in thread_create function is not set
     *
     * @return void*
     */
    virtual VOID_T run();

    /**
     * @fn delay
     * @brief
     *
     */
    static void delay(unsigned int ms, unsigned int us = 0);
};
} // namespace ipc::core
#endif // CTHREAD_H