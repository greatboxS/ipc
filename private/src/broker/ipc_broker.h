#ifndef __IPC_BROKER_H__
#define __IPC_BROKER_H__

#include "ipc_type.h"
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <atomic>

namespace gbs
{
    namespace ipc
    {
        class IpcMessage;
        class IpcProfile;

        class IpcBrokerBase
        {
        private:
            std::shared_mutex m_mtx;
            std::thread *m_brokerThread;
            std::atomic<bool> m_running;

            void brokerMainLoop();

        protected:
            virtual int onBrokerStart() = 0;
            virtual int onBrokerStop() = 0;
            virtual int onBrokerRunning() {}
            virtual int onBrokerSend(const IpcProfile *profile, const std::string &message) = 0;
            virtual int onBrokerSubscribe(const IpcProfile *profile, const std::string &topic) = 0;
            virtual int onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic) = 0;
            virtual int onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) = 0;

        public:
            IpcBrokerBase();
            virtual ~IpcBrokerBase() {}

            virtual int start();
            virtual int stop();

            virtual int subscribe(const IpcProfile *profile, const std::string &topic, IpcTopicCallback fnc = NULL);
            virtual int unsubscribe(const IpcProfile *profile, const std::string &topic);
            virtual int publicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message);
            virtual int sendMessage(const IpcProfile *profile, const std::string &message);
        };
    } // namespace ipc
} // namespace gbs

#endif // __IPC_BROKER_H__