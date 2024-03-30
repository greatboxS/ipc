/**
 * @file Queue.h
 * @author greatboxs (greatboxs@greatboxs.com)
 * @brief This Queue (FIFO) class using with shared memory or
 *        allocated memory.
 * @version 0.1
 * @date 2022-08-15
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __QUEUE_H__
#define __QUEUE_H__

#include "common/Typedef.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

#define QUEUE_BUFF_OFFSET 128
#define QUEUE_INITIALIZED 0xFF

class __DLL_DECLSPEC__ Queue {
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
     * @brief Construct a new Queue object
     *
     * @param virt
     */
    Queue(void *virt, size_t mesgsize, size_t mesgcount);

    /**
     * @brief Destroy the Queue object
     *
     */
    ~Queue() { delete[] m_pstBufferNodes; }

    /**
     * @fn GetRequiredSize
     * @brief Get the Required Size of queue
     *
     * @param msgsize   Message size
     * @param msgcount  Total message will be stored
     * @return size_t
     */
    static size_t GetRequiredSize(size_t msgsize, size_t msgcount) {
        size_t size = QUEUE_BUFF_OFFSET + (sizeof(Queue::BufferHeader_t) * msgcount) + msgsize * msgcount;
        return size;
    }

    /**
     * @fn PushBack
     * @brief PushBack a new message into queue
     *
     * @param buff  Poiter to data to be writen to queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 Queue is full
     *
     */
    int PushBack(const char *buff, size_t size);

    /**
     * @fn PushFront
     * @brief PushFront a new message into queue
     *
     * @param buff  Poiter to data to be writen to queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 Queue is full
     *
     */
    int PushFront(const char *buff, size_t size);

    /**
     * @fn Front
     * @brief Get front base address
     *
     * @param size  Data size
     * @return void *
     *
     */
    void *Front(size_t *size = NULL);

    /**
     * @fn Back
     * @brief Get Back base address
     *
     * @param size  Data size
     * @return void *
     *
     */
    void *Back(size_t *size = NULL);

    /**
     * @fn Front
     * @brief Read a message from queue
     *
     * @param buff  Poiter to data to be read out from queue
     * @param size  Data size
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 Queue is empty
     *
     */
    int Front(char *buff, size_t size);

    /**
     * @fn Pop
     * @brief Pop a message from queue
     *
     * @return int  0 Success
     *              -1 Not initialized
     *              -2 Queue is empty
     *
     */
    int Pop();

    /**
     */
    int Clear();

    /**
     * @fn Size
     * @brief Get current queue size
     *
     * @return size_t
     */
    size_t Size() { return std::abs((int)m_pstQueueHeader->s32WIndex - (int)m_pstQueueHeader->s32RIndex); }

    /**
     * @fn GetMemSize
     * @brief Get queue memory size
     *
     * @return size_t
     */
    size_t GetMemSize() { return m_pstQueueHeader->u32TotalSize; }

    /**
     * @fn Initialized
     * @brief
     *
     * @return int
     */
    int Initialized() { return (int)(m_pstQueueHeader->s32Inited == QUEUE_INITIALIZED); }

    /**
     * @fn IsFull
     * @brief
     *
     * @return int
     */
    int IsFull() { return m_pstQueueHeader->s32Full; }

    /**
     * @fn IsEmpty
     * @brief
     *
     * @return int
     */
    int IsEmpty() { return (Size() == 0); }

    /**
     * @fn MessageSize
     * @brief
     *
     * @return size_t
     */
    size_t MessageSize() { return m_pstQueueHeader->u32Msgsize; }

    /**
     * @fn MessageCount
     * @brief
     *
     * @return size_t
     */
    size_t MessageCount() { return m_pstQueueHeader->u32Msgcount; }
};

#endif // __QUEUE_H__