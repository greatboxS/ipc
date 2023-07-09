/**
 * @file CMessageQueue.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "common/Typedef.h"
#include "osal/OSAL.h"

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CMessageQueue
        {
        private:
            MSGQ_T m_stMsgq;

        public:
            CMessageQueue() {}
            ~CMessageQueue() {}

            int Open(const char *name);
            int Create(const char *name, size_t msgsize, size_t msgcount);
            int Send(const char *buff, size_t size);
            int Receive(char *buff, size_t size);
            int Size();
            int Close();
            int Release();
        };
    }; // namespace osac
} // namespace gbs