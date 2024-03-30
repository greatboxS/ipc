/**
 * @file CThread.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __CTHREAD_H__
#define __CTHREAD_H__

#include "common/Typedef.h"
#include "osal/OSAL.h"
#include <functional>
#include <memory>

#define DEFAULT_WAIT_TIME 10000U

namespace gbs {
namespace osac {
using CThreadCallbackFunction = std::function<void *(void *)>;

class __DLL_DECLSPEC__ CThread {
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
        CThread *t;
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
     * @fn Handle
     * @brief ThreadHandler
     *
     * @param param
     * @return TASK_RET
     */
    static TASK_RET Handle(VOID_T param);
    void Wrapper() {
        Run();
        if (m_pCallback) m_pCallback(m_pParam);
    }

public:
    CThread();
    virtual ~CThread();

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
    int Create(const char *name, CThreadCallbackFunction callback = NULL, void *param = NULL, int priority = 0, int core = -1);

    /**
     * @fn Join
     * @brief Join current thread
     *
     * @return int
     */
    int Join();

    /**
     * @fn Detach
     * @brief Detach current thread
     *
     * @return int
     */
    int Detach();

    /**
     * @fn Start
     * @brief Start current thread
     *
     * @return int
     */
    int Start();

    /**
     * @fn Stop
     * @brief Stop current thread
     *
     * @return int
     */
    int Stop();

    /**
     * @fn Finalize
     * @brief Finalize current thread
     *
     * @return int
     */
    int Finalize();

    /**
     * @fn WaitForCompletion
     * @brief Wait for thread to be completed after finalize is called
     *
     * @param ms
     * @return bool true if completion, otherwise false
     */
    bool WaitForCompletion(uint64_t ms = DEFAULT_WAIT_TIME);

    /**
     * @fn Terminate
     * @brief RequestFinalize
     *
     * @return int
     */
    int Terminate();

    /**
     * @fn OnThreadInitialize
     * @brief Onetime calling function, used to the initialization
     *
     * @return int
     */
    virtual int OnThreadInitialize() { return 0; }

    /**
     * @fn OnThreadFinalize
     * @brief On thread finalize
     *
     * @return int
     */
    virtual int OnThreadFinalize() { return 0; }

    /**
     * @fn Run
     * @brief Thread Run function, user have to overwrite this function
     * 		  if callback in THREAD_Create function is not set
     *
     * @return void*
     */
    virtual VOID_T Run();

    /**
     * @fn Delay
     * @brief
     *
     */
    static void Delay(unsigned int ms, unsigned int us = 0);
};
}; // namespace osac
} // namespace gbs
#endif // __CTHREAD_H__