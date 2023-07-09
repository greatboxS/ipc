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
#include "queue/Queue.h"

#include <stdio.h>
#include <string.h>

namespace gbs
{
    namespace osal
    {

#define GENERATE_MSGQ_NAME(from)          \
    char genName[MSG_QUEUE_NAME_LEN + 4]; \
    memset(genName, 0, sizeof(genName));  \
    snprintf(genName, sizeof(genName), "msgq.%s", from);

        /**
         * @fn MSGQ_Open
         * @brief Open existing message
         *
         * @param msgq   Message queue information structure
         * @param name      Message queue name
         * @param msgsize   Message queue size (each message)
         * @param msgcount  Message queue maximum of message in queue
         * @return int      0 if success, otherwise -1
         */
        int MSGQ_Open(MSGQ_T &msgq, const char *name) {
            SEM_T sem;
            MUTEX_T mtx;
            size_t size = 0;
            int ret = 0;

            if (!name) {
                _EXCEPT_THROW("MSGQ_Open error, invalid name!");
            }

            if (SEM_Open(sem, name) != 0) {
                LOG_OSAL_ERROR("[%s] Open semaphore failed\n", __FUNCTION__);
                SHM_Close(msgq.shm);
                return RET_ERR;
            }

            if (MUTEX_Create(mtx, name) != 0) {
                LOG_OSAL_ERROR("[%s] Open mutex %s failed\n", __FUNCTION__, name);
                SEM_Close(sem);
                return RET_ERR;
            }

            if (SHM_Open(msgq.shm, name, QUEUE_BUFF_OFFSET) < 0) {
                LOG_OSAL_ERROR("[%s] Open shared memory failed\n", __FUNCTION__);
                SEM_Close(sem);
                MUTEX_Destroy(mtx);
                return RET_ERR;
            }

            if (SEM_Wait(sem) != RET_OK) return RET_ERR;

            if (MUTEX_Lock(mtx) != RET_OK) {
                SEM_Post(sem);
                return RET_ERR;
            }

            Queue queue(msgq.shm.virt, 0, 0);
            size = queue.GetMemSize();
            SHM_Close(msgq.shm);
            SHM_Destroy(msgq.shm);

            if (SHM_Open(msgq.shm, name, size) < 0) {
                LOG_OSAL_ERROR("[%s] Open shared memory failed\n", __FUNCTION__);
                ret = -1;
            }

            if (ret == RET_OK) {
                msgq.sem = sem;
                msgq.mtx = mtx;
                msgq.que = new Queue(msgq.shm.virt, 0, 0);
                msgq.msgcount = msgq.que->MessageCount();
                msgq.msgsize = msgq.que->MessageSize();
                strncpy(msgq.mqname, name, sizeof(msgq.mqname));
            }
            MUTEX_UnLock(mtx);
            SEM_Post(sem);

            if (ret != RET_OK) {
                SEM_Close(sem);
                MUTEX_Destroy(mtx);
            }

            return ret;
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
            size_t size = Queue::GetRequiredSize(msgsize, msgcount);
            SEM_T sem;
            MUTEX_T mtx;

            if (!name) {
                _EXCEPT_THROW("MSGQ_Create error, invalid name!");
            }

            if (SEM_Create(sem, SEM_DEFAULT_INIT_VALUE, name) != 0) {
                LOG_OSAL_ERROR("[%s] Create semaphore failed\n", __FUNCTION__);
                return RET_ERR;
            }

            if (MUTEX_Create(mtx, name) != 0) {
                LOG_OSAL_ERROR("[%s] Create mutex failed\n", __FUNCTION__);
                SEM_Close(sem);
                SEM_Destroy(sem);
                return RET_ERR;
            }

            if (SHM_Create(msgq.shm, name, size) < 0) {
                LOG_OSAL_ERROR("[%s] Create shared memory failed\n", __FUNCTION__);
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

            LOG_OSAL_INFO("[%s] Create message queue %s success\n", __FUNCTION__, name);
            return RET_OK;
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
        }

        /**
         * @fn MSGQ_Receive
         * @brief Receive message that
         *
         * @param msgq   Message queue file descriptor
         * @param buff      Data
         * @param size      Total bytes to be read (it has to be equal to mesgsize)
         * @return int      0 if success, otherwise -1
         */
        int MSGQ_Send(MSGQ_T &msgq, const char *buff, size_t size) {
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
        }

        /**
         * @fn MSGQ_GetCurrSize
         * @brief Get current message queue size
         *
         * @param msgq 	Message queue data structure
         * @return int
         */
        int MSGQ_GetCurrSize(MSGQ_T &msgq) {
            int ret = SEM_Wait(msgq.sem);
            if (ret != RET_OK) return RET_ERR;
            ret = MUTEX_Lock(msgq.mtx);
            if (ret != RET_OK) {
                SEM_Post(msgq.sem);
                return RET_ERR;
            }

            ret = (int)msgq.que->Size();
            MUTEX_UnLock(msgq.mtx);
            SEM_Post(msgq.sem);
            return ret;
        }

        /**
         * @fn MSGQ_Close
         * @brief Close message queue file descriptor
         *
         * @param msgq   Message queue data structure
         * @return int      0 if sucess, otherwise error
         */
        int MSGQ_Close(MSGQ_T &msgq) {
            MUTEX_Destroy(msgq.mtx);
            SHM_Close(msgq.shm);
            SEM_Close(msgq.sem);
            return RET_OK;
        }

        /**
         * @fn MSGQ_Destroy
         * @brief Release message queue (if no process links with message queue, it will be released)
         *
         * @param msgq   Message queue data structure
         * @return int      0 if sucess, otherwise error
         */
        int MSGQ_Destroy(MSGQ_T &msgq) {
            SHM_Destroy(msgq.shm);
            SEM_Destroy(msgq.sem);
            delete msgq.que;
            msgq.que = NULL;
            return RET_OK;
        }
    } // namespace osal
} // namespace gbs