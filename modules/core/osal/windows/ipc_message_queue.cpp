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

#include "osal/ipc_message_queue.h"
#include "osal/ipc_mutex.h"
#include "osal/ipc_semaphore.h"
#include "osal/ipc_shared_memory.h"
#include "queue/Queue.h"

#include <stdio.h>
#include <string.h>

namespace ipc::core {

#define GENERATE_MSGQ_NAME(from)          \
    char genName[MSG_QUEUE_NAME_LEN + 4]; \
    memset(genName, 0, sizeof(genName));  \
    snprintf(genName, sizeof(genName), "msgq.%s", from);

/**
 * @fn mesgqueue_open
 * @brief Open existing message
 *
 * @param msgq   Message queue information structure
 * @param name      Message queue name
 * @param msgsize   Message queue size (each message)
 * @param msgcount  Message queue maximum of message in queue
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_open(MSGQ_T &msgq, const char *name) {
    SEM_T sem;
    MUTEX_T mtx;
    size_t size = 0;
    int ret = 0;

    if (!name) {
        OSAL_ERR("[%s] Invalid arguments\n", __FUNCTION__);
        return RET_ERR;
    }

    if (semaphore_open(sem, name) != 0) {
        OSAL_ERR("[%s] Open semaphore failed\n", __FUNCTION__);
        shared_mem_close(msgq.shm);
        return RET_ERR;
    }

    if (mutex_create(mtx, name) != 0) {
        OSAL_ERR("[%s] Open mutex %s failed\n", __FUNCTION__, name);
        semaphore_close(sem);
        return RET_ERR;
    }

    if (shared_mem_open(msgq.shm, name, QUEUE_BUFF_OFFSET) < 0) {
        OSAL_ERR("[%s] Open shared memory failed\n", __FUNCTION__);
        semaphore_close(sem);
        mutex_destroy(mtx);
        return RET_ERR;
    }

    if (semaphore_wait(sem) != RET_OK) return RET_ERR;

    if (mutex_lock(mtx) != RET_OK) {
        semaphore_post(sem);
        return RET_ERR;
    }

    Queue queue(msgq.shm.virt, 0, 0);
    size = queue.get_mem_size();
    shared_mem_close(msgq.shm);
    shared_mem_destroy(msgq.shm);

    if (shared_mem_open(msgq.shm, name, size) < 0) {
        OSAL_ERR("[%s] Open shared memory failed\n", __FUNCTION__);
        ret = -1;
    }

    if (ret == RET_OK) {
        msgq.sem = sem;
        msgq.mtx = mtx;
        msgq.que = new Queue(msgq.shm.virt, 0, 0);
        msgq.msgcount = msgq.que->message_count();
        msgq.msgsize = msgq.que->message_size();
        strncpy(msgq.mqname, name, sizeof(msgq.mqname));
    }
    mutex_unlock(mtx);
    semaphore_post(sem);

    if (ret != RET_OK) {
        semaphore_close(sem);
        mutex_destroy(mtx);
    }

    return ret;
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
    size_t size = Queue::get_required_size(msgsize, msgcount);
    SEM_T sem;
    MUTEX_T mtx;

    if (!name) {
        _EXCEPT_THROW("mesgqueue_create error, invalid name!");
    }

    if (semaphore_create(sem, SEM_DEFAULT_INIT_VALUE, name) != 0) {
        OSAL_ERR("[%s] Create semaphore failed\n", __FUNCTION__);
        return RET_ERR;
    }

    if (mutex_create(mtx, name) != 0) {
        OSAL_ERR("[%s] Create mutex failed\n", __FUNCTION__);
        semaphore_close(sem);
        semaphore_destroy(sem);
        return RET_ERR;
    }

    if (shared_mem_create(msgq.shm, name, size) < 0) {
        OSAL_ERR("[%s] Create shared memory failed\n", __FUNCTION__);
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

    OSAL_INFO("[%s] Create message queue %s success\n", __FUNCTION__, name);
    return RET_OK;
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
}

/**
 * @fn mesgqueue_receive
 * @brief Receive message that
 *
 * @param msgq   Message queue file descriptor
 * @param buff      Data
 * @param size      Total bytes to be read (it has to be equal to mesgsize)
 * @return int      0 if success, otherwise -1
 */
int mesgqueue_send(MSGQ_T &msgq, const char *buff, size_t size) {
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
}

/**
 * @fn mesgqueue_get_current_size
 * @brief Get current message queue size
 *
 * @param msgq 	Message queue data structure
 * @return int
 */
int mesgqueue_get_current_size(MSGQ_T &msgq) {
    int ret = semaphore_wait(msgq.sem);
    if (ret != RET_OK) return RET_ERR;
    ret = mutex_lock(msgq.mtx);
    if (ret != RET_OK) {
        semaphore_post(msgq.sem);
        return RET_ERR;
    }

    ret = (int)msgq.que->size();
    mutex_unlock(msgq.mtx);
    semaphore_post(msgq.sem);
    return ret;
}

/**
 * @fn mesgqueue_close
 * @brief Close message queue file descriptor
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int mesgqueue_close(MSGQ_T &msgq) {
    mutex_destroy(msgq.mtx);
    shared_mem_close(msgq.shm);
    semaphore_close(msgq.sem);
    return RET_OK;
}

/**
 * @fn mesgqueue_destroy
 * @brief Release message queue (if no process links with message queue, it will be released)
 *
 * @param msgq   Message queue data structure
 * @return int      0 if sucess, otherwise error
 */
int mesgqueue_destroy(MSGQ_T &msgq) {
    shared_mem_destroy(msgq.shm);
    semaphore_destroy(msgq.sem);
    delete msgq.que;
    msgq.que = NULL;
    return RET_OK;
}
} // namespace ipc::core