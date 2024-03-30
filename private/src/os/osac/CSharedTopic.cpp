#include "CSharedTopic.h"
#include "dbg/Debug.h"
#include "osal/Mutex.h"
#include "osal/Semaphore.h"
#include "osal/SharedMemory.h"

namespace gbs
{
    namespace osac
    {
        /**
         * @brief Construct a new CSharedTopic object
         *
         * @param name
         * @param size
         */
        CSharedTopic::CSharedTopic(const char *name, size_t size) :
            m_strName(std_str(name)),
            m_szSize(size),
            m_IsOpen(0) {
            memset(m_strLastModifier, 0, sizeof(m_strLastModifier));
        }

        /**
         * @fn Open
         * @brief Open a shared topic
         *
         * @return int  0 if success, -1 if failed
         */
        int CSharedTopic::Open() {
            int ret = 0;
            MUTEX_T mtx;
            SEM_T sem;
            SHM_T shm;

            if (osal::SEM_Create(sem, SEM_DEFAULT_INIT_VALUE, m_strName.c_str()) != 0) {
                CLOG_ERROR("SEM_Create %s failed\n", m_strName.c_str());
                return RET_ERR;
            }

            if (osal::MUTEX_Create(mtx, m_strName.c_str()) != 0) {
                CLOG_ERROR("MUTEX_Create %s failed\n", m_strName.c_str());
                osal::SEM_Close(sem);
                osal::SEM_Destroy(sem);
                return RET_ERR;
            }

            if (osal::SHM_Create(shm, m_strName.c_str(), m_szSize + sizeof(SharedTopicHeader_t)) < 0) {
                CLOG_ERROR("SHM_Create %s failed\n", m_strName.c_str());
                osal::SEM_Close(sem);
                osal::SEM_Destroy(sem);
                osal::MUTEX_Destroy(mtx);
                return RET_ERR;
            }

            m_stSem = sem;
            m_stMtx = mtx;
            m_stShm = shm;
            m_pstSharedTopicHeader = static_cast<SharedTopicHeader_t *>(m_stShm.virt);

            /* Topic is creating */
            if (m_pstSharedTopicHeader->u32Code != SHARED_TOPIC_IDENTIFY) {
                CLOG_INFO("Create new shared topic: %s\n", m_strName.c_str());
                memset(m_stShm.virt, 0, m_stShm.size);
                m_pstSharedTopicHeader->u32Code = SHARED_TOPIC_IDENTIFY;
                m_pstSharedTopicHeader->u32TransactionId = 0;
                m_pstSharedTopicHeader->u32TransactionSize = 0;
                m_pstSharedTopicHeader->u32Size = m_szSize;
                memset(m_pstSharedTopicHeader->strModifierInfo, 0, sizeof(m_pstSharedTopicHeader->strModifierInfo));
                m_u32PrevTransactionId = m_pstSharedTopicHeader->u32TransactionId;
            } else {
                if (Lock() >= 0) {
                    m_u32PrevTransactionId = 0;
                    m_szSize = m_pstSharedTopicHeader->u32Size;
                    CLOG_INFO("Open shared topic: %s, u32TransactionId = %d\n", m_strName.c_str(),
                              m_pstSharedTopicHeader->u32TransactionId);
                    UnLock();
                }
            }
            m_IsOpen = 1;
            return 0;
        }

        /**
         * @fn Close()
         * @brief Close shared topic
         *
         * @return int
         */
        int CSharedTopic::Close() {
            osal::MUTEX_Destroy(m_stMtx);
            osal::SHM_Close(m_stShm);
            osal::SEM_Close(m_stSem);
            return 0;
        }

        /**
         * @fn Read
         * @brief Read data from this topic
         *
         * @param buff 		Pointer to buffer
         * @param size 		Maximum of bytes to read
         * @return int
         */
        int CSharedTopic::Read(char *buff, size_t size) {
            int ret = -1;
            if (!buff || size <= 0 || m_szSize < size || !IsOpen()) {
                _EXCEPT_THROW("CSharedTopic Read error, invalid arguments!");
            }
            if (Lock() >= 0) {
                memcpy(buff, &m_pstSharedTopicHeader->buff[0], m_pstSharedTopicHeader->u32TransactionSize);
                ret = (int)m_pstSharedTopicHeader->u32TransactionSize;
                MarkAsRead();
                UnLock();
            }
            return ret;
        }

        /**
         * @fn GetLastModifier
         * @brief Get the last modifier
         *
         * @return int
         */
        const char *CSharedTopic::GetLastModifier() {
            if (!IsOpen()) {
                _EXCEPT_THROW("CSharedTopic GetLastModifier error, not open!");
            }
            if (Lock() >= 0) {
                memcpy(m_strLastModifier, m_pstSharedTopicHeader->strModifierInfo, SHARED_TOPIC_MODIFIER_NAME_LENGTH);
                UnLock();
            }
            return m_strLastModifier;
        }

        /**
         * @fn SetLastModifier
         * @brief Set the last modifier
         *
         * @return int
         */
        int CSharedTopic::SetLastModifier(const char *modifier) {
            if (!IsOpen()) {
                _EXCEPT_THROW("CSharedTopic SetLastModifier error, invalid arguments!");
            }
            if (Lock() >= 0) {
                memcpy(m_pstSharedTopicHeader->strModifierInfo, modifier, SHARED_TOPIC_MODIFIER_NAME_LENGTH);
                UnLock();
                return 0;
            }
            return RET_ERR;
        }

        /**
         * @fn Write
         * @brief Write data to this topic
         *
         * @param buff 		Pointer to buffer
         * @param size 		Maximum of bytes to write
         * @return int
         */
        int CSharedTopic::Write(const char *buff, size_t size) {
            int ret = -1;
            if (!buff || size <= 0 || m_szSize < size || !IsOpen()) {
                _EXCEPT_THROW("CSharedTopic Write error, invalid arguments!");
            }
            if (Lock() >= 0) {
                memcpy(&m_pstSharedTopicHeader->buff[0], buff, size);
                m_pstSharedTopicHeader->u32TransactionSize = static_cast<uint32_t>(size);
                NotifyDataChanged();
                ret = static_cast<int>(size);
                UnLock();
            }
            return ret;
        }

        /**
         * @fn IsDataChanged
         * @brief Check if topic data is updated
         *
         * @return int  0 if no changes is made, otherwise bytes of changes
         */
        int CSharedTopic::IsDataChanged() {
            int ret = 0;
            if (!IsOpen()) {
                _EXCEPT_THROW("CSharedTopic IsDataChanged error, not open!");
            }
            if (Lock() >= 0) {
                if (m_pstSharedTopicHeader->u32TransactionId != m_u32PrevTransactionId) {
                    ret = (int)m_pstSharedTopicHeader->u32TransactionSize;
                }
                UnLock();
            }
            return ret;
        }

        /**
         * @fn MarkAsRead
         * @brief Topic data changed is ignored after this function
         *		  Lock() and Unlock() is required
         * @return int
         */
        int CSharedTopic::MarkAsRead() {
            m_u32PrevTransactionId = m_pstSharedTopicHeader->u32TransactionId;
            return 0;
        }

        /**
         * @fn NotifyDataChanged
         * @brief Set the Data Changed object
         *
         * @return int
         */
        int CSharedTopic::NotifyDataChanged() {
            m_pstSharedTopicHeader->u32TransactionId++;
            return 0;
        }

        /**
         * @fn Lock
         * @brief
         *
         * @return int
         */
        int CSharedTopic::Lock() {
            if (osal::SEM_Wait(m_stSem) != RET_OK) return RET_ERR;

            if (osal::MUTEX_Lock(m_stMtx) != RET_OK) {
                osal::SEM_Post(m_stSem);
                return RET_ERR;
            }
            return RET_OK;
        }

        /**
         * @fn Unlock
         * @brief
         *
         * @return int
         */
        int CSharedTopic::UnLock() {
            osal::MUTEX_UnLock(m_stMtx);
            osal::SEM_Post(m_stSem);
            return 0;
        }

        /**
         * @fn GetBaseAddress
         * @brief Get the Base Address object
         *
         * @return void*
         */
        void *CSharedTopic::GetBaseAddress() {
            if (!IsOpen()) {
                _EXCEPT_THROW("CSharedTopic GetBaseAddress error, not open!");
            }
            return &m_pstSharedTopicHeader->buff[0];
        }
    } // namespace osac
} // namespace gbs
