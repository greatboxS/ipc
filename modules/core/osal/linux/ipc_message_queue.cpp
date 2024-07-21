/**
 * @file ipc_message_queue.cpp
 * @author kha.le (greatboxs@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "osal/ipc_message_queue.h"
#include "osal/ipc_mutex.h"
#include "osal/ipc_semaphore.h"
#include "osal/ipc_shared_memory.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace ipc::core {

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
 * @fn mesgqueue_open
 * @brief Open existing message
 *
 * @param name      Message queue name
 * @param msgq   	Message queue information structure
 * @param msgsize   Message queue size (each message)
 * @param msgcount  Message queue maximum of message in queue
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_open(MSGQ_T &msgq, const char *name) {

    GENERATE_MSGQ_NAME(name);
    OSAL_INFO("[%s] Open message queue name %s\n", __FUNCTION__, genName);

#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    SEM_T sem;
    MUTEX_T mtx;
    size_t size = 0;
    int ret = 0;

    if (semaphore_open(sem, genName) != 0) {
        OSAL_ERR("%s: Open semaphore %s faile\n", __FUNCTION__, name);
        return RET_ERR;
    }

    if (mutex_create(mtx, genName) != 0) {
        OSAL_ERR("%s: Open mutex %s failed\n", __FUNCTION__, name);
        semaphore_close(sem);
        return RET_ERR;
    }

    if (shared_mem_open(msgq.shm, genName, QUEUE_BUFF_OFFSET) != RET_OK) {
        OSAL_ERR("%s: Open shared memory %s failed\n", __FUNCTION__, name);
        semaphore_close(sem);
        mutex_destroy(mtx);
        return RET_ERR;
    }

    ret = semaphore_wait(sem);
    if (ret != RET_OK) return RET_ERR;
    ret = mutex_lock(mtx);
    if (ret != RET_OK) {
        semaphore_post(sem);
        return RET_ERR;
    }

    Queue que(msgq.shm.virt, 0, 0);
    size = que.get_mem_size();
    shared_mem_close(msgq.shm);
    shared_mem_destroy(msgq.shm);

    if (shared_mem_open(msgq.shm, genName, size) != RET_OK) {
        OSAL_ERR("mesgqueue_open: Open shared memory %s failed\n", name);
        ret = RET_ERR;
    }

    if (ret == RET_OK) {
        msgq.sem = sem;
        msgq.mtx = mtx;
        msgq.que = new Queue(msgq.shm.virt, 0, 0);
        msgq.msgsize = msgq.que->message_size();
        msgq.msgcount = msgq.que->message_count();
        strncpy(msgq.mqname, name, sizeof(msgq.mqname));
    }
    mutex_unlock(mtx);
    semaphore_post(sem);

    if (ret == RET_OK) return RET_OK;

    semaphore_close(sem);
    mutex_destroy(mtx);
    return RET_ERR;
#else
    struct mq_attr attr;
    int fd = 0;

    memset(&attr, 0, sizeof(attr));

    if ((fd = mq_open(genName, O_RDWR | BLOCKING_FLAG)) != RET_OK) {
        OSAL_ERR("[%s] mq_open() failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }

    if (mq_getattr(fd, &attr) != RET_OK) {
        OSAL_ERR("[%s] mq_getattr() failed %s\n", __FUNCTION__, __ERROR_STR__);
        if (close(fd) != RET_OK) {
            OSAL_ERR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
        }
        return RET_ERR;
    }

    msgq.handle = fd;
    memcpy(msgq.mqname, name, sizeof(msgq.mqname));
    msgq.msgsize = std_str(attr).mq_msgsize;
    msgq.msgcount = std_str(attr).mq_maxmsg;
    msgq.currcount = std_str(attr).mq_curmsgs;
    OSAL_INFO("[%s] Message info: msgsize = %ld, msgcount = %ld\n", __FUNCTION__, attr.mq_msgsize,
                  attr.mq_maxmsg);

    return RET_OK;
#endif
}

/**
 * @fn mesgqueue_create
 * @brief Create new or open existing message
 *
 * @param msgq   	Message queue information structure
 * @param name      Message queue name
 * @param msgsize   Message queue size (each message)
 * @param msgcount  Message queue maximum of message in queue
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_create(MSGQ_T &msgq, const char *name, size_t msgsize, size_t msgcount) {

    GENERATE_MSGQ_NAME(name);
    OSAL_INFO("[%s] Create message queue name %s\n", __FUNCTION__, genName);

#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    size_t size = Queue::get_required_size(msgsize, msgcount);
    SEM_T sem;
    MUTEX_T mtx;

    if (semaphore_create(sem, SEM_DEFAULT_INIT_VALUE, genName) != 0) {
        OSAL_ERR("%s: semaphore_create %s failed\n", __FUNCTION__, name);
        return RET_ERR;
    }

    if (mutex_create(mtx, genName) != 0) {
        OSAL_ERR("%s: mutex_create %s failed\n", __FUNCTION__, name);
        semaphore_close(sem);
        semaphore_destroy(sem);
        return RET_ERR;
    }

    if (shared_mem_create(msgq.shm, genName, size) != RET_OK) {
        OSAL_ERR("%s: shared_mem_create %s failed\n", __FUNCTION__, name);
        semaphore_close(sem);
        semaphore_destroy(sem);
        mutex_destroy(mtx);
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

    ret = mesgqueue_open(msgq, name);
    if (ret == RET_OK) return RET_OK;

    OSAL_ERR("[%s] Message queue %s doesn't exist, Create new one\n", __FUNCTION__, name);
    if ((fd = mq_open(genName, O_CREAT, MSGQ_MODE, &attr)) != RET_OK) {
        OSAL_ERR("[%s] Create message queue %s failed %s\n", __FUNCTION__, name, __ERROR_STR__);
        return RET_ERR;
    }
    if (close(fd) != RET_OK) {
        OSAL_ERR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
    }

    return mesgqueue_open(msgq, name);
#endif
}

/**
 * @fn mesgqueue_receive
 * @brief Receive message that
 *
 * @param msgq      Message queue file descriptor
 * @param buff      Data
 * @param size      Total bytes to be read (it has to be equal to mesgsize)
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_receive(MSGQ_T &msgq, char *buff, size_t size) {
    if (!buff) {
        OSAL_ERR("[%s] buff is null\n", __FUNCTION__);
        return RET_ERR;
    }
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = semaphore_wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = mutex_lock(msgq.mtx);
    if (ret != RET_OK) {
        semaphore_post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->front(buff, size);
    msgq.que->pop();
    mutex_unlock(msgq.mtx);
    semaphore_post(msgq.sem);
    return ret;
#else
    return mq_receive(msgq.handle, buff, size, DEFAULT_MSGQ_PRIORITY);
#endif
}

/**
 * @fn mesgqueue_receive
 * @brief Receive message that
 *
 * @param msgq      Message queue file descriptor
 * @param buff      Data
 * @param size      Total bytes to be read (it has to be equal to mesgsize)
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_send(MSGQ_T &msgq, const char *buff, size_t size) {
    if (!buff) {
        OSAL_ERR("[%s] buff is null\n", __FUNCTION__);
        return RET_ERR;
    }
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = semaphore_wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = mutex_lock(msgq.mtx);
    if (ret != RET_OK) {
        semaphore_post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->push_back(buff, size);
    mutex_unlock(msgq.mtx);
    semaphore_post(msgq.sem);
    return ret;
#else
    return mq_send(msgq.handle, buff, size, DEFAULT_MSGQ_PRIORITY);
#endif
}

/**
 * @fn mesgqueue_get_current_size
 * @brief Get current message queue size
 *
 * @param msgq 	Message queue data structure
 * @return int
 */
int mesgqueue_get_current_size(MSGQ_T &msgq) {
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    int ret = semaphore_wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = mutex_lock(msgq.mtx);
    if (ret != RET_OK) {
        semaphore_post(msgq.sem);
        return RET_ERR;
    }

    ret = msgq.que->size();
    mutex_unlock(msgq.mtx);
    semaphore_post(msgq.sem);
    return ret;
#else
    struct mq_attr attr;
    if (mq_getattr(msgq.handle, &attr) != RET_OK) {
        OSAL_ERR("[%s] mq_getattr() failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    msgq.currcount = std_str(attr).mq_curmsgs;
    return attr.mq_curmsgs;
#endif
}

/**
 * @fn mesgqueue_close
 * @brief Close message queue file descriptor
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int mesgqueue_close(MSGQ_T &msgq) {
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    semaphore_close(msgq.sem);
    shared_mem_close(msgq.shm);
    return RET_OK;
#else
    if (close(msgq.handle) != RET_OK) {
        OSAL_ERR("[%s] close(fd) failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
#endif
}

/**
 * @fn mesgqueue_destroy
 * @brief Release message queue (if no process links with message queue, it will be released)
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int mesgqueue_destroy(MSGQ_T &msgq) {
    GENERATE_MSGQ_NAME(msgq.mqname);
#ifdef SHARED_MEMORY_MESSAGE_QUEUE
    semaphore_destroy(msgq.sem);
    mutex_destroy(msgq.mtx);
    shared_mem_destroy(msgq.shm);
    delete msgq.que;
    msgq.que = NULL;
#else
    if (mq_unlink(genName) != RET_OK) {
        OSAL_ERR("[%s] mq_unlink(genName) failed %s\n", __FUNCTION__, __ERROR_STR__);
        return RET_ERR;
    }
    return RET_OK;
#endif
}
} // namespace ipc::core