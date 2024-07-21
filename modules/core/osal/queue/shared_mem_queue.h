#ifndef SHARED_MEM_QUEUE_H
#define SHARED_MEM_QUEUE_H

/**
 * @file shared_mem_queue.h
 * @author greatboxs (greatboxs@greatboxs.com)
 * @brief This shared_mem_queue (FIFO) class using with shared memory or
 *        allocated memory.
 * @version 0.1
 * @date 2022-08-15
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "../osal.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

#define QUEUE_BUFF_OFFSET 128
#define QUEUE_INITIALIZED 0xFF

class __dll_declspec__ shared_mem_queue {
public:
    typedef struct __QueueHeader_t {
        int32_t s32Id;
        uint32_t u32Msgsize;
        uint32_t u32Msgcount;
        int32_t s32RIndex;
        int32_t s32WIndex;
        int32_t s32Inited;
        int32_t s32Full;
        int32_t s32CurrentSize;
        uint32_t u32TotalSize;
    } QueueHeader_t;

    typedef struct __BufferHeader_t {
        uint32_t u32Offset;
        uint32_t u32Size;
        uint32_t u32Maxsize;
    } BufferHeader_t;

    typedef struct __BufferNode_t {
        void *pAddr;
        uint32_t *pu32Size;
        uint32_t *pu32Maxsize;
    } BufferNode_t;

private:
    long long m_llBaseAddr;
    long long m_llBodyAddr;
    QueueHeader_t *m_pstQueueHeader;
    BufferNode_t *m_pstBufferNodes;
    BufferHeader_t *m_pstBufferHeader;

public:
    /**
     * @brief Construct a new shared_mem_queue object
     *
     * @param virt
     */
    shared_mem_queue(void *virt, size_t mesgsize, size_t mesgcount);

    /**
     * @brief destroy the shared_mem_queue object
     *
     */
    ~shared_mem_queue() { delete[] m_pstBufferNodes; }

    /**
     * @fn get_required_size
     * @brief Get the Required size of queue
     *
     * @param msgsize   Message size
     * @param msgcount  Total message will be stored
     * @return size_t
     */
    static size_t get_required_size(size_t msgsize, size_t msgcount) {
        size_t size = QUEUE_BUFF_OFFSET + (sizeof(shared_mem_queue::BufferHeader_t) * msgcount) + msgsize * msgcount;
        return size;
    }

    /**
     * @fn push_back
     * @brief push_back a new message into queue
     *
     * @param buff  Poiter to data to be writen to queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 shared_mem_queue is full
     *
     */
    int push_back(const char *buff, size_t size);

    /**
     * @fn push_front
     * @brief push_front a new message into queue
     *
     * @param buff  Poiter to data to be writen to queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 shared_mem_queue is full
     *
     */
    int push_front(const char *buff, size_t size);

    /**
     * @fn front
     * @brief Get front base address
     *
     * @param size  Data size
     * @return void *
     *
     */
    void *front(size_t *size = NULL);

    /**
     * @fn back
     * @brief Get back base address
     *
     * @param size  Data size
     * @return void *
     *
     */
    void *back(size_t *size = NULL);

    /**
     * @fn front
     * @brief Read a message from queue
     *
     * @param buff  Poiter to data to be read out from queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 shared_mem_queue is empty
     *
     */
    int front(char *buff, size_t size);

    /**
     * @fn pop
     * @brief pop a message from queue
     *
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 shared_mem_queue is empty
     *
     */
    int pop();

    /**
     */
    int clear();

    /**
     * @fn size
     * @brief Get current queue size
     *
     * @return size_t
     */
    size_t size() { return std::abs((int)m_pstQueueHeader->s32WIndex - (int)m_pstQueueHeader->s32RIndex); }

    /**
     * @fn get_mem_size
     * @brief Get queue memory size
     *
     * @return size_t
     */
    size_t get_mem_size() { return m_pstQueueHeader->u32TotalSize; }

    /**
     * @fn is_initialized
     * @brief
     *
     * @return int
     */
    int is_initialized() { return (int)(m_pstQueueHeader->s32Inited == QUEUE_INITIALIZED); }

    /**
     * @fn full
     * @brief
     *
     * @return int
     */
    int full() { return m_pstQueueHeader->s32Full; }

    /**
     * @fn empty
     * @brief
     *
     * @return int
     */
    int empty() { return (size() == 0); }

    /**
     * @fn message_size
     * @brief
     *
     * @return size_t
     */
    size_t message_size() { return m_pstQueueHeader->u32Msgsize; }

    /**
     * @fn message_count
     * @brief
     *
     * @return size_t
     */
    size_t message_count() { return m_pstQueueHeader->u32Msgcount; }
};

#endif // SHARED_MEM_QUEUE_H