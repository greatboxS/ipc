
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

#include "Queue.h"
#include "dbg/Debug.h"
#include <math.h>
#include <string.h>

/**
 * @brief Construct a new Queue object
 *
 * @param virt
 */
Queue::Queue(void *virt, size_t mesgsize, size_t mesgcount) {
	m_llBaseAddr = (long long)virt;
	m_pstQueueHeader = (QueueHeader_t *)m_llBaseAddr;
	m_pstBufferHeader = (BufferHeader_t *)(m_llBaseAddr + QUEUE_BUFF_OFFSET);

	CLOG_TRACE("Mapping to virtual address %llx, msgsize = %zu, msgcount = %zu\n", m_llBaseAddr, mesgsize, mesgcount);

	if (!Initialized()) {
		m_llBodyAddr = (long long)m_pstBufferHeader + (sizeof(BufferHeader_t) * mesgcount);
		m_pstQueueHeader->u32Msgsize = static_cast<uint32_t>(mesgsize);
		m_pstQueueHeader->u32Msgcount = static_cast<uint32_t>(mesgcount);
		m_pstQueueHeader->s32CurrentSize = 0;
		m_pstQueueHeader->s32RIndex = 0;
		m_pstQueueHeader->s32WIndex = 0;
		m_pstQueueHeader->s32Full = 0;
		m_pstQueueHeader->s32Inited = QUEUE_INITIALIZED;
		m_pstQueueHeader->u32TotalSize = static_cast<uint32_t>(Queue::GetRequiredSize(mesgsize, mesgcount));

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
int Queue::PushBack(const char *buff, size_t size) {
	if (!Initialized()) return -1;
	if (IsFull()) {
		CLOG_ERROR("Buffer is full, %zd\n", Size());
		return -2;
	}

	int ret = 0;
	BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32WIndex];

	if (!buff || size <= 0 || size > (*node->pu32Maxsize)) {
		_EXCEPT_THROW("Queue PushBack error, invalid arguments!");
	}

	memcpy(node->pAddr, buff, size);
	*(node->pu32Size) = static_cast<uint32_t>(size);
	m_pstQueueHeader->s32WIndex = (++m_pstQueueHeader->s32WIndex) % m_pstQueueHeader->u32Msgcount;

	if (m_pstQueueHeader->s32WIndex == m_pstQueueHeader->s32RIndex) {
		m_pstQueueHeader->s32Full = 1;
	}
	ret = (int)size;
	return ret;
}

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
int Queue::PushFront(const char *buff, size_t size) {
	if (!Initialized()) return -1;
	if (IsFull()) {
		CLOG_ERROR("Buffer is full, %zu\n", Size());
		return -2;
	}

	int ret = 0;
	BufferNode_t *node = NULL;
	size_t que_size = Size();
	size_t rIndex = 0;

	if (que_size > 0) {
		if (m_pstQueueHeader->s32RIndex <= 0) {
			rIndex = m_pstQueueHeader->u32Msgcount - 1;
		} else {
			rIndex = m_pstQueueHeader->s32RIndex - 1;
		}
	}
	node = &m_pstBufferNodes[rIndex];

	if (!buff || size <= 0 || size > (*node->pu32Maxsize)) {
		_EXCEPT_THROW("Queue PushFront error, invalid arguments!");
	}

	memcpy(node->pAddr, buff, size);
	m_pstQueueHeader->s32RIndex = static_cast<int32_t>(rIndex);
	*(node->pu32Size) = static_cast<uint32_t>(size);
	ret = (int)size;
	if (m_pstQueueHeader->s32WIndex == m_pstQueueHeader->s32RIndex) {
		m_pstQueueHeader->s32Full = 1;
	}
	return ret;
}

/**
 * @fn Front
 * @brief Get front base address
 *
 * @param size  Data size
 * @return void *
 *
 */
void *Queue::Front(size_t *size) {
	if (!Initialized()) return NULL;
	if (IsEmpty()) return NULL;

	int ret = 0;
	BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32RIndex];
	if (size)
		*size = *(node->pu32Size);
	return node->pAddr;
}

/**
 * @fn Back
 * @brief Get back base address
 *
 * @param size  Data size
 * @return void *
 *
 */
void *Queue::Back(size_t *size) {
	if (!Initialized()) return NULL;
	if (IsEmpty()) return NULL;

	int ret = 0;
	int index = (m_pstQueueHeader->s32WIndex > 0 ? m_pstQueueHeader->s32WIndex : (m_pstQueueHeader->u32Msgcount - 1));
	BufferNode_t *node = &m_pstBufferNodes[index];
	if (size)
		*size = *(node->pu32Size);
	return node->pAddr;
}

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
int Queue::Front(char *buff, size_t size) {
	int ret = 0;
	BufferNode_t *node = &m_pstBufferNodes[m_pstQueueHeader->s32RIndex];
	if (!buff || size <= 0 || size > (*node->pu32Maxsize)) {
		_EXCEPT_THROW("Queue Front error, invalid arguments!");
	}
	memcpy(buff, node->pAddr, *(node->pu32Size));
	ret = (int)*(node->pu32Size);
	return ret;
}

/**
 * @fn Pop
 * @brief Pop a message from queue
 *
 * @return int  0 Success
 *              -1 Not initialized
 *              -2 Queue is empty
 *
 */
int Queue::Pop() {
	if (!Initialized()) return -1;
	if (IsEmpty()) return -2;
	m_pstQueueHeader->s32Full = 0;
	m_pstQueueHeader->s32RIndex = (++m_pstQueueHeader->s32RIndex) % m_pstQueueHeader->u32Msgcount;
	return 0;
}

/**
 * @fn Clear
 * @brief clear all message
 *
 * @return int
 */
int Queue::Clear() {
	m_pstQueueHeader->s32WIndex = 0;
	m_pstQueueHeader->s32RIndex = 0;
	m_pstQueueHeader->s32Full = 0;
	return 0;
}
