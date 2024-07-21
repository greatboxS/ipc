#include "osal/ipc_thread.h"
#include <pthread.h>
#include <string.h>
#include <unistd.h>

namespace ipc::core {
/**
 * @fn thread_create
 * @brief Create new thread using pthread library
 *
 * @param name		Thread name
 * @param fnc 		Thread function call
 * @param param 	Parameters
 * @param priority 	Thread priority
 * @param core 		CPU that created thread will be attached to
 * @return TASK_T 	Thread handler ( this TASK_T is allocated by thread_create(), it will be
 * 					deallocated by thread_terminate() )
 */
int thread_create(THREAD_T &threadInfo, const char *name, THREAD_FunctionCall fnc, VOID_T param, int priority, int core) {
    pthread_t *pthread = NULL;
    pthread_attr_t attr;
    cpu_set_t cpuset;
    int ret = -1;
    int current_policy = sched_getscheduler(getpid());
    int max_priority = sched_get_priority_max(current_policy);
    int min_priority = sched_get_priority_min(current_policy);

#ifdef CONFIG_PTHREAD_SCHEDULE
    max_priority = sched_get_priority_max(PTHREAD_SCHEDULE_POLICY);
    min_priority = sched_get_priority_min(PTHREAD_SCHEDULE_POLICY);

    /** Schedule policy
     * SCHED_OTHER		0
     * SCHED_FIFO		1
     * SCHED_RR			2
     */
    int policy = PTHREAD_SCHEDULE_POLICY;
    int inheritsched = PTHREAD_EXPLICIT_SCHED;
    struct sched_param sched_pr;
#endif

    OSAL_INFO("Schedule policy: %s, priority: (%d - %d)\n",
           (current_policy == 0 ? "SCHED_OTHER" : (current_policy == 1 ? "SCHED_FIFO" : (current_policy == 2 ? "SCHED_RR" : "UNKNOWN"))),
           min_priority, max_priority);

    if (priority > max_priority)
        priority = max_priority;

    if (priority < min_priority)
        priority = min_priority;

    pthread = new pthread_t;
    if (pthread == nullptr) {
        return RET_ERR;
    }
    memset(pthread, 0, sizeof(pthread_t));

    /* Assigns default values */
    if ((ret = pthread_attr_init(&attr)) != 0) {
        OSAL_ERR("[%s] Attribute init failed, %d\n", __FUNCTION__, ret);
    }

#ifdef CONFIG_PTHREAD_SCHEDULE
    if (current_policy < 0) {
        OSAL_ERR("[%s] sched_getscheduler, failed, %d\n", __FUNCTION__, __ERROR__);
    } else if (current_policy != policy) {
        OSAL_INFO("[%s] sched_getscheduler, scuccess, %d\n", __FUNCTION__, current_policy);

        if ((ret = pthread_attr_setinheritsched(&attr, inheritsched)) < 0) {
            OSAL_ERR("[%s] pthread_attr_setinheritsched, failed, %d\n", __FUNCTION__, ret);
        } else {
            OSAL_INFO("[%s] pthread_attr_setinheritsched, success\n", __FUNCTION__);
        }

        if ((ret = pthread_attr_setschedpolicy(&attr, policy)) != 0) {
            OSAL_ERR("[%s] pthread_attr_setschedpolicy, failed, %d\n", __FUNCTION__, ret);
        } else {
            OSAL_INFO("[%s] pthread_attr_setschedpolicy, success\n", __FUNCTION__);
        }

        sched_pr.sched_priority = priority;
        if (pthread_attr_setschedparam(&attr, &sched_pr) != 0) {
            OSAL_ERR("[%s] Set sched priority failed\n");
        }
    }
#endif

    if ((ret = pthread_create(pthread, &attr, fnc, param)) != 0) {
        OSAL_ERR("[%s] Create pthread failed, %d\n", __FUNCTION__, ret);
        goto create_error;
    }

    if (name) {
        if ((ret = pthread_setname_np(*pthread, name)) != 0) {
            OSAL_ERR("[%s] Set pthread name %s failed, %d\n", __FUNCTION__, name, ret);
        } else {
            OSAL_INFO("[%s] Set pthread name %s success\n", __FUNCTION__, name);
        }
    }

    if ((ret = pthread_setschedprio(*pthread, priority)) != 0) {
        OSAL_ERR("[%s] pthread_setschedprio failed, %d\n", __FUNCTION__, ret);
    } else {
        OSAL_INFO("[%s pthread_setschedprio %d success\n", __FUNCTION__, priority);
    }

    // Enable pthread cancelation
    {
        int old = 0;
        if ((ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &old)) != 0) {
            OSAL_ERR("[%s] pthread_setcancelstate failed, %d\n", __FUNCTION__, ret);
        }
        if ((ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &old)) != 0) {
            OSAL_ERR("[%s] pthread_setcanceltype failed, %d\n", __FUNCTION__, ret);
        }
    }

    // Marking created thread to specific cpu core
    if (core >= 0) {
        /* Set affinity mask to include CPUs (core) */
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);

        ret = pthread_setaffinity_np(*pthread, sizeof(cpuset), &cpuset);
        if (ret != 0) {
            OSAL_ERR("[%s] pthread_setaffinity_np failed, %d\n", __FUNCTION__, ret);
        }
        /* Check the actual affinity mask assigned to the pthread. */
        ret = pthread_getaffinity_np(*pthread, sizeof(cpuset), &cpuset);
        if (ret != 0) {
            OSAL_ERR("[%s] pthread_getaffinity_np failed, %d\n", __FUNCTION__, ret);
        }

        OSAL_INFO("[%s] Set returned by pthread_getaffinity_np() contained:\n", __FUNCTION__);
        for (int j = 0; j < CPU_SETSIZE; j++)
            if (CPU_ISSET(j, &cpuset))
                OSAL_INFO("[%s]    CPU %d\n", __FUNCTION__, j);
    }

    threadInfo.fnc = fnc;
    threadInfo.core = core;
    threadInfo.priority = priority;
    threadInfo.task = pthread;
    strncpy(threadInfo.name, name, sizeof(threadInfo.name));
    return RET_OK;

create_error:
    delete pthread;
    pthread_attr_destroy(&attr);
    return RET_ERR;
}

/**
 * @fn thread_join
 * @brief Wait for thread return
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_join(THREAD_T &threadInfo) {
    return pthread_join(*threadInfo.task, NULL);
}

/**
 * @fn thread_detach
 * @brief Detach a thread, the resource of this thread automatically
 *        free when application is terminated
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_detach(THREAD_T &threadInfo) {
    return pthread_detach(*threadInfo.task);
}

/**
 * @fn thread_run
 * @brief
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_run(THREAD_T &threadInfo) {
    return RET_OK;
}

/**
 * @fn thread_suspend
 * @brief Suspend thread (WINDOWS)
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_suspend(THREAD_T &threadInfo) {
    return RET_OK;
}

/**
 * @fn thread_terminate
 * @brief Terminate a thread intermediately
 *
 * @param threadInfo Thread info structure
 * @return int
 */
int thread_terminate(THREAD_T &threadInfo) {
    int ret = pthread_cancel(*threadInfo.task);
    delete threadInfo.task;
    return ret;
}

/**
 * @fn thread_delay
 * @brief Delay ms and us
 *
 * @param ms	Delay in ms
 * @param us	Delay in us
 * @return int
 */
int thread_delay(unsigned int ms, unsigned int us) {
    if (ms)
        usleep(ms * 1000);
    if (us)
        usleep(us);
    return RET_OK;
}
} // namespace ipc::core