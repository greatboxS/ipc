/**
 * @file CSemaphore.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __CSEMAPHORE_H__
#define __CSEMAPHORE_H__

#include "common/Typedef.h"
#include "osal/OSAL.h"

namespace gbs
{
    namespace osac
    {
        class __DLL_DECLSPEC__ CSemaphore
        {
        private:
            SEM_T m_stSem;

        public:
            CSemaphore();
            ~CSemaphore();

            int Create(int value, const char *name = NULL);
            int Open(const char *name);
            int Wait();
            int Post();
            int Value();
            int Close();
            int Destroy();
        };
    }; // namespace osac
} // namespace gbs
#endif // __CSEMAPHORE_H__