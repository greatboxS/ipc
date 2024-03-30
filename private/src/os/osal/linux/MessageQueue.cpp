/**
 * @file MessageQueue.cpp
 * @author kha.le (greatboxs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "osal/MessageQueue.h"
#include "common/Typedef.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include "osal/Semaphore.h"
#include "osal/SharedMemory.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace gbs {
namespace osal {

/* Use in nonblocking mode */
#ifdef MSGQ_NONBLOCKING_MODE
#define BLOCKING_FLAG O_NONBLOCK
#else
#define BLOCKING_FLAG 0
#endif

#define DEFAULT_MSGQ_PRIORITY 0
#define MSGQ_MODE             (S_IRUSR | S_IWUSR)

#define GENERATE_MSGQ_NAME(from)                            \
    char genName[MSG_QUEUE_NAME_LEN + 4];                   \
    memset(genName, 0, sizeof(genName));                    \
    if (from[0] == '/') {                                   \
        snprintf(genName, sizeof(genName), "%s_mq", from);  \
    } else {                                                \
        snprintf(genName, sizeof(genName), "/%s_mq", from); \
    }

/**
 * @fn MSGQ_Open
 * @brief Open existing message
 *
 * @param name      Message queue name
 * @param msgq   	Message queue information structure
 * @param msgsize   Message queue size (each message)
 * @param msgcount  Message queue maximum of message in queue
 * @return int      0 if success, otherwise -1
 */
int MSGQ_Open(MSGQ_T &msgq, const char *name) {

    GENERATE_MSGQ_NAME(name);
    LOG_OSAL_INFO("[%s] Open message queue name %s\n", __FUNCTION__, genName);

#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    SEM_T sem;
    MUTEX_T mtx;
    size_t size = 0;
    int ret = 0;

    if (SEM_Open(sem, genName) != 0) {
        LOG_OSAL_ERROR("%s: Open semaphore %s faile\n", __FUNCTION__, name);
        return RET_ERR;
    }

    if (MUTEX_Create(mtx, genName) != 0) {
        LOG_OSAL_ERROR("%s: Open mutex %s failed\n", __FUNCTION__, name);
        SEM_Close(sem);
        return RET_ERR;
    }

    if (SHM_Open(msgq.shm, genName, QUEUE_BUFF_OFFSET) != RET_OK) {
        LOG_OSAL_ERROR("%s: Open shared memory %s failed\n", __FUNCTION__, name);
        SEM_Close(sem);
        MUTEX_Destroy(mtx);
        return RET_ERR;
    }

    ret = SEM_Wait(sem);
    if (ret != RET_OK) return RET_ERR;
    ret = MUTEX_Lock(mtx);
    if (ret != RET_OK) {
        SEM_Post(sem);
        return RET_ERR;
    }

    Queue que(msgq.shm.virt, 0, 0);
    size = que.GetMemSize();
    SHM_Close(msgq.shm);
    SHM_Destroy(msgq.shm);

    if (SHM_Open(msgq.shm, genName, size) != RET_OK) {
        LOG_OSAL_ERROR("MSGQ_Open: Open shared memory %s failed\n", name);
        ret = RET_ERR;
    }

    if (ret == RET_OK) {
        msgq.sem = sem;
        msgq.mtx = mtx;
        msgq.que = new Queue(msgq.shm.virt, 0, 0);
        msgq.msgsize = msgq.que->MessageSize();
        msgq.msgcount = msgq.que->MessageCount();
        strncpy(msgq.mqname, name, sizeof(msgq.mqname));
    }
    MUTEX_UnLock(mtx);
    SEM_Post(sem);

    if (ret == RET_OK) return RET_OK;

    SEM_Close(sem);
    MUTEX_Destroy(mtx);
    return RET_ERR;
#else
    struct mq_attr attr;
    int fd = 0;

    memset(&attr, 0, sizeof(attr));

    if ((fd = mq_open(genName, O_RDWR | BLOCKING_FLAG)) != RET_OK) {
        LOG_OSAL_ERROR("[%s] mq_open() failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    if (mq_getattr(fd, &attr) != RET_OK) {
        LOG_OSAL_ERROR("[%s] mq_getattr() failed %s\n", __FUNCTION__, __ERROR_STR__);
        if (close(fd) != RET_OK) {
            LOG_OSAL_ERROR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
        return RET_ERR;
    }

    msgq.handle = fd;
    memcpy(msgq.mqname, name, sizeof(msgq.mqname));
    msgq.msgsize = std_str(attr).mq_msgsize;
    msgq.msgcount = std_str(attr).mq_maxmsg;
    msgq.currcount = std_str(attr).mq_curmsgs;
    LOG_OSAL_INFO("[%s] Message info: msgsize = %ld, msgcount = %ld\n", __FUNCTION__, attr.mq_msgsize,
                  attr.mq_maxmsg);

    return RET_OK;
#endif
}

/**
 * @fn MSGQ_Create
 * @brief Create new or open existing message
 *
 * @param msgq   	Message queue information structure
 * @param name      Message queue name
 * @param msgsize   Message queue size (each message)
 * @param msgcount  Message queue maximum of message in queue
 * @return int      0 if success, otherwise -1
 */
int MSGQ_Create(MSGQ_T &msgq, const char *name, size_t msgsize, size_t msgcount) {

    GENERATE_MSGQ_NAME(name);
    LOG_OSAL_INFO("[%s] Create message queue name %s\n", __FUNCTION__, genName);

#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    size_t size = Queue::GetRequiredSize(msgsize, msgcount);
    SEM_T sem;
    MUTEX_T mtx;

    if (SEM_Create(sem, SEM_DEFAULT_INIT_VALUE, genName) != 0) {
        LOG_OSAL_ERROR("%s: SEM_Create %s failed\n", __FUNCTION__, name);
        return RET_ERR;
    }

    if (MUTEX_Create(mtx, genName) != 0) {
        LOG_OSAL_ERROR("%s: MUTEX_Create %s failed\n", __FUNCTION__, name);
        SEM_Close(sem);
        SEM_Destroy(sem);
        return RET_ERR;
    }

    if (SHM_Create(msgq.shm, genName, size) != RET_OK) {
        LOG_OSAL_ERROR("%s: SHM_Create %s failed\n", __FUNCTION__, name);
        SEM_Close(sem);
        SEM_Destroy(sem);
        MUTEX_Destroy(mtx);
        return RET_ERR;
    }

    msgq.msgsize = msgsize;
    msgq.msgcount = msgcount;
    msgq.sem = sem;
    msgq.mtx = mtx;
    msgq.que = new Queue(msgq.shm.virt, msgsize, msgcount);
    strncpy(msgq.mqname, name, sizeof(msgq.mqname));
    return RET_OK;
#else
    int ret = 0;
    int fd = 0;
    struct mq_attr attr;
    memset(&attr, 0, sizeof(attr));

    attr.mq_maxmsg = msgcount;
    attr.mq_msgsize = msgsize;

    ret = MSGQ_Open(msgq, name);
    if (ret == RET_OK) return RET_OK;

    LOG_OSAL_ERROR("[%s] Message queue %s doesn't exist, Create new one\n", __FUNCTION__, name);
    if ((fd = mq_open(genName, O_CREAT, MSGQ_MODE, &attr)) != RET_OK) {
        LOG_OSAL_ERROR("[%s] Create message queue %s failed %s\n", __FUNCTION__, name, __ERROR_STR__);
        return RET_ERR;
    }
    if (close(fd) != RET_OK) {
        LOG_OSAL_ERROR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
    }

    return MSGQ_Open(msgq, name);
#endif
}

/**
 * @fn MSGQ_Receive
 * @brief Receive message that
 *
 * @param msgq      Message queue file descriptor
 * @param buff      Data
 * @param size      Total bytes to be read (it has to be equal to mesgsize)
 * @return int      0 if success, otherwise -1
 */
int MSGQ_Receive(MSGQ_T &msgq, char *buff, size_t size) {
    if (!buff) {
        LOG_OSAL_ERROR("[%s] buff is null\n", __FUNCTION__);
        return RET_ERR;
    }
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = SEM_Wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = MUTEX_Lock(msgq.mtx);
    if (ret != RET_OK) {
        SEM_Post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->Front(buff, size);
    msgq.que->Pop();
    MUTEX_UnLock(msgq.mtx);
    SEM_Post(msgq.sem);
    return ret;
#else
    return mq_receive(msgq.handle, buff, size, DEFAULT_MSGQ_PRIORITY);
#endif
}

/**
 * @fn MSGQ_Receive
 * @brief Receive message that
 *
 * @param msgq      Message queue file descriptor
 * @param buff      Data
 * @param size      Total bytes to be read (it has to be equal to mesgsize)
 * @return int      0 if success, otherwise -1
 */
int MSGQ_Send(MSGQ_T &msgq, const char *buff, size_t size) {
    if (!buff) {
        LOG_OSAL_ERROR("[%s] buff is null\n", __FUNCTION__);
        return RET_ERR;
    }
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = SEM_Wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = MUTEX_Lock(msgq.mtx);
    if (ret != RET_OK) {
        SEM_Post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->PushBack(buff, size);
    MUTEX_UnLock(msgq.mtx);
    SEM_Post(msgq.sem);
    return ret;
#else
    return mq_send(msgq.handle, buff, size, DEFAULT_MSGQ_PRIORITY);
#endif
}

/**
 * @fn MSGQ_GetCurrSize
 * @brief Get current message queue size
 *
 * @param msgq 	Message queue data structure
 * @return int
 */
int MSGQ_GetCurrSize(MSGQ_T &msgq) {
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = SEM_Wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = MUTEX_Lock(msgq.mtx);
    if (ret != RET_OK) {
        SEM_Post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->Size();
    MUTEX_UnLock(msgq.mtx);
    SEM_Post(msgq.sem);
    return ret;
#else
    struct mq_attr attr;
    if (mq_getattr(msgq.handle, &attr) != RET_OK) {
        LOG_OSAL_ERROR("[%s] mq_getattr() failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    msgq.currcount = std_str(attr).mq_curmsgs;
    return attr.mq_curmsgs;
#endif
}

/**
 * @fn MSGQ_Close
 * @brief Close message queue file descriptor
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int MSGQ_Close(MSGQ_T &msgq) {
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    SEM_Close(msgq.sem);
    SHM_Close(msgq.shm);
    return RET_OK;
#else
    if (close(msgq.handle) != RET_OK) {
        LOG_OSAL_ERROR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
#endif
}

/**
 * @fn MSGQ_Destroy
 * @brief Release message queue (if no process links with message queue, it will be released)
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int MSGQ_Destroy(MSGQ_T &msgq) {
    GENERATE_MSGQ_NAME(msgq.mqname);
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    SEM_Destroy(msgq.sem);
    MUTEX_Destroy(msgq.mtx);
    SHM_Destroy(msgq.shm);
    delete msgq.que;
    msgq.que = NULL;
#else
    if (mq_unlink(genName) != RET_OK) {
        LOG_OSAL_ERROR("[%s] mq_unlink(genName) failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
#endif
}
} // namespace osal
} // namespace gbs