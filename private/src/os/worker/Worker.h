#ifndef __WORKER_H__
#define __WORKER_H__

#include "common/Typedef.h"
#include <functional>

#define DEFAULT_WORKER_TERMINATION_TIMEOUT 15000

namespace gbs
{
    namespace osac
    {
        typedef std::function<void *(void *)> WorkFunction;

        class __DLL_DECLSPEC__ IWorker
        {
        public:
            virtual ~IWorker() {}
            virtual int OnWorkerInitialize() = 0;
            virtual int OnWorkerFinalize() = 0;
            virtual int OnWorkerRun(void *param, size_t len) = 0;
            virtual int OnRequestWorkerStart() = 0;
            virtual int OnRequestWorkerStop() = 0;
        };

        class __DLL_DECLSPEC__ INotify
        {
        public:
            virtual int OnNotify(void *) = 0;
        };

        typedef enum {
            WORKER_INIT,
            WORKER_FINAL,
            WORKER_RUN,
            WORKER_STOP,
            WORKER_PRE_EXIT,
            WORKER_EXIT_DONE,
        } eWorkerState;
    } // namespace osac
} // namespace gbs
#endif // __WORKER_H__