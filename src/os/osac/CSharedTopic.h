#ifndef __CSHAREDTOPIC_H__
#define __CSHAREDTOPIC_H__

#include "osal/OSAL.h"
#include <string>

#define SHARED_TOPIC_IDENTIFY             (0x5588U)
#define SHARED_TOPIC_MODIFIER_NAME_LENGTH 64U
namespace gbs
{
    namespace osac
    {
        class CSharedTopic
        {
            struct SharedTopicHeader_t {
                uint32_t u32Code;
                uint32_t u32Size;
                uint32_t u32TransactionId;
                uint32_t u32TransactionSize;
                uint32_t s32ModifierId;
                char strModifierInfo[SHARED_TOPIC_MODIFIER_NAME_LENGTH];
                char buff[1];
            };

        private:
            std::string m_strName;
            char m_strLastModifier[SHARED_TOPIC_MODIFIER_NAME_LENGTH];
            uint32_t m_u32PrevTransactionId;
            size_t m_szSize;
            int m_IsOpen;
            MUTEX_T m_stMtx;
            SHM_T m_stShm;
            SEM_T m_stSem;
            SharedTopicHeader_t *m_pstSharedTopicHeader;

        public:
            /**
             * @brief Construct a new CSharedTopic object
             *
             * @param name
             * @param size
             */
            CSharedTopic(const char *name, size_t size);
            ~CSharedTopic() {}

            /**
             * @fn Open
             * @brief Open a shared topic
             *
             * @return int  0 if success, -1 if failed
             */
            int Open();

            /**
             * @fn Close()
             * @brief Close shared topic
             *
             * @return int
             */
            int Close();

            /**
             * @fn GetName
             * @brief Get topic name
             *
             * @return const char *
             */
            const char *GetName() { return m_strName.c_str(); }

            /**
             * @fn GetSize
             * @brief Get the Size object
             *
             * @return size_t
             */
            size_t GetSize() { return m_szSize; }

            /**
             * @fn IsOpen
             * @brief
             *
             * @return int
             */
            int IsOpen() { return m_IsOpen; }

            /**
             * @fn Read
             * @brief Read data from this topic
             *		  No Lock() and UnLock() is required

             * @param buff 		Pointer to buffer
             * @param size 		Maximum of bytes to read
             * @return int
             */
            int Read(char *buff, size_t size);

            /**
             * @fn GetLastModifier
             * @brief Get the last modifier
             *		  No Lock() and UnLock() is required
             *
             * @return const char *
             */

            const char *GetLastModifier();
            /**
             * @fn SetLastModifier
             * @brief Set the last modifier
             * 		  No Lock() and UnLock() is required
             *
             * @return int
             */
            int SetLastModifier(const char *modifier);

            /**
             * @fn Write
             * @brief Write data to this topic
             *		  No Lock() and UnLock() is required

             * @param buff 		Pointer to buffer
             * @param size 		Maximum of bytes to write
             * @return int
             */
            int Write(const char *buff, size_t size);

            /**
             * @fn IsDataChanged
             * @brief Check if topic data is updated
             * 		  No Lock() and UnLock() is required
             *
             * @return int  0 if no changes is made, otherwise the bytes of changes is returned
             */
            int IsDataChanged();

            /**
             * @fn MarkAsRead
             * @brief Topic data changed is ignored after this function
             *		  Lock() and Unlock() is required
             * @return int
             */
            int MarkAsRead();

            /**
             * @fn NotifyDataChanged
             * @brief Set the Data Changed object
             * 		  Lock() and Unlock() is required
             *
             * @return int
             */
            int NotifyDataChanged();

            /**
             * @fn Lock
             * @brief
             *
             * @return int
             */
            int Lock();

            /**
             * @fn Unlock
             * @brief
             *
             * @return int
             */
            int UnLock();

            /**
             * @fn GetBaseAddress
             * @brief Get the Base Address object
             *
             * @return void*
             */
            void *GetBaseAddress();
        };

    } // namespace osac
} // namespace gbs
#endif // __CSHAREDTOPIC_H__