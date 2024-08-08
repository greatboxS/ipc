
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

#include "shared_mem_queue.h"
#include <math.h>
#include <string.h>
#include <cassert>
/**
 * @brief Construct a new shared_mem_queue object
 *
 * @param virt
 */
shared_mem_queue::shared_mem_queue(void *virt, size_t mesgsize, size_t mesgcount) {
    m_llBaseAddr = (long long)virt;
    m_pstQueueHeader = (QueueHeader_t *)m_llBaseAddr;
    m_pstBufferHeader = (BufferHeader_t *)(m_llBaseAddr + QUEUE_BUFF_OFFSET);

    OSAL_INFO("Mapping to virtual address %llx, msgsize = %zu, msgcount = %zu\n", m_llBaseAddr, mesgsize, mesgcount);

    if (!is_initialized()) {
        m_llBodyAddr = (long long)m_pstBufferHeader + (sizeof(BufferHeader_t) * mesgcount);
        m_pstQueueHeader->u32Msgsize = static_cast<uint32_t>(mesgsize);
        m_pstQueueHeader->u32Msgcount = static_cast<uint32_t>(mesgcount);
        m_pstQueueHeader->s32CurrentSize = 0;
        m_pstQueueHeader->s32RIndex = 0;
        m_pstQueueHeader->s32WIndex = 0;
        m_pstQueueHeader->s32Full = 0;
        m_pstQueueHeader->s32Inited = QUEUE_INITIALIZED;
        m_pstQueueHeader->u32TotalSize = static_cast<uint32_t>(shared_mem_queue::get_required_size(mesgsize, mesgcount));

        for (unsigned int i = 0; i < mesgcount; i++) {
            m_pstBufferHeader[i].u32Offset = static_cast<uint32_t>(i * mesgsize);
            m_pstBufferHeader[i].u32Size = 0;
            m_pstBufferHeader[i].u32Maxsize = static_cast<uint32_t>(mesgsize);
        }
    } else {
        m_llBodyAddr = (long long)m_pstBufferHeader + (sizeof(BufferHeader_t) * m_pstQueueHeader->u32Msgcount);
    }

    m_pstBufferNodes = new BufferNode_t[m_pstQueueHeader->u32Msgcount];
    assert(m_pstBufferNodes);

    /* Update list of queue buffer address */
    for (int i = 0; i < (int)m_pstQueueHeader->u32Msgcount; i++) {
        m_pstBufferNodes[i].pAddr = (void *)(m_llBodyAddr + m_pstBufferHeader[i].u32Offset);
        m_pstBufferNodes[i].pu32Maxsize = &m_pstBufferHeader[i].u32Maxsize;
        m_pstBufferNodes[i].pu32Size = &m_pstBufferHeader[i].u32Size;
    }
}

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
int shared_mem_queue::push_back(const char *buff, size_t _size) {
    if (!is_initialized()) {
        return -1;
    }
    if (full()) {
        OSAL_ERR("Buffer is full, %zd\n", size());
        return -2;
    }

    int ret = 0;
    BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32WIndex];

    if (!buff || _size <= 0 || _size > (*node->pu32Maxsize)) {
        OSAL_ERR("shared_mem_queue push_back error, invalid arguments!");
        return -1;
    }

    memcpy(node->pAddr, buff, _size);
    *(node->pu32Size) = static_cast<uint32_t>(_size);
    m_pstQueueHeader->s32WIndex = (++m_pstQueueHeader->s32WIndex) % m_pstQueueHeader->u32Msgcount;

    if (m_pstQueueHeader->s32WIndex == m_pstQueueHeader->s32RIndex) {
        m_pstQueueHeader->s32Full = 1;
    }
    ret = (int)_size;
    return ret;
}

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
int shared_mem_queue::push_front(const char *buff, size_t _size) {
    if (!is_initialized()) {
        return -1;
    }
    if (full()) {
        OSAL_ERR("Buffer is full, %zu\n", size());
        return -2;
    }

    int ret = 0;
    BufferNode_t *node = NULL;
    size_t que_size = size();
    size_t rIndex = 0;

    if (que_size > 0) {
        if (m_pstQueueHeader->s32RIndex <= 0) {
            rIndex = m_pstQueueHeader->u32Msgcount - 1;
        } else {
            rIndex = m_pstQueueHeader->s32RIndex - 1;
        }
    }
    node = &m_pstBufferNodes[rIndex];

    if (!buff || _size <= 0 || _size > (*node->pu32Maxsize)) {
        OSAL_ERR("shared_mem_queue push_front error, invalid arguments!");
        return -1;
    }

    memcpy(node->pAddr, buff, _size);
    m_pstQueueHeader->s32RIndex = static_cast<int32_t>(rIndex);
    *(node->pu32Size) = static_cast<uint32_t>(_size);
    ret = (int)_size;
    if (m_pstQueueHeader->s32WIndex == m_pstQueueHeader->s32RIndex) {
        m_pstQueueHeader->s32Full = 1;
    }
    return ret;
}

/**
 * @fn front
 * @brief Get front base address
 *
 * @param size  Data size
 * @return void *
 *
 */
void *shared_mem_queue::front(size_t *size) {
    if (!is_initialized()) {
        return nullptr;
    }
    if (empty()) {
        return nullptr;
    }

    int ret = 0;
    BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32RIndex];
    if (size) {
        *size = *(node->pu32Size);
    }
    return node->pAddr;
}

/**
 * @fn back
 * @brief Get back base address
 *
 * @param size  Data size
 * @return void *
 *
 */
void *shared_mem_queue::back(size_t *size) {
    if (!is_initialized()) {
        return nullptr;
    }
    if (empty()) {
        return nullptr;
    }

    int ret = 0;
    int index = (m_pstQueueHeader->s32WIndex > 0 ? m_pstQueueHeader->s32WIndex : (m_pstQueueHeader->u32Msgcount - 1));
    BufferNode_t *node = &m_pstBufferNodes[index];
    if (size) {
        *size = *(node->pu32Size);
    }
    return node->pAddr;
}

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
int shared_mem_queue::front(char *buff, size_t size) {
    int ret = 0;
    BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32RIndex];
    if (!buff || size <= 0 || size > (*node->pu32Maxsize)) {
        OSAL_ERR("shared_mem_queue front error, invalid arguments!");
        return -1;
    }
    memcpy(buff, node->pAddr, *(node->pu32Size));
    ret = (int)*(node->pu32Size);
    return ret;
}

/**
 * @fn pop
 * @brief pop a message from queue
 *
 * @return int  0 Success
 *              -1 Not initialized
 *              -2 shared_mem_queue is empty
 *
 */
int shared_mem_queue::pop() {
    if (!is_initialized()) {
        return -1;
    }
    if (empty()) {
        return -2;
    }
    m_pstQueueHeader->s32Full = 0;
    m_pstQueueHeader->s32RIndex = (++m_pstQueueHeader->s32RIndex) % m_pstQueueHeader->u32Msgcount;
    return 0;
}

/**
 * @fn clear
 * @brief clear all message
 *
 * @return int
 */
int shared_mem_queue::clear() {
    m_pstQueueHeader->s32WIndex = 0;
    m_pstQueueHeader->s32RIndex = 0;
    m_pstQueueHeader->s32Full = 0;
    return 0;
}
