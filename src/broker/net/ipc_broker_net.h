#ifndef __IPC_BROKER_NET_H__
#define __IPC_BROKER_NET_H__

#include "ipc_broker.h"

namespace gbs
{
    namespace ipc
    {
        class IpcProfile;

        class IpcNetBroker : public IpcBrokerBase
        {
            const IpcProfile *m_profile;

        private:
            virtual int onBrokerStart();
            virtual int onBrokerStop();
            virtual int onBrokerSend(const IpcProfile *profile, const std::string &message);
            virtual int onBrokerSubscribe(const IpcProfile *profile, const std::string &topic);
            virtual int onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic);
            virtual int onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message);

        public:
            explicit IpcNetBroker(const IpcProfile *profile);
            virtual ~IpcNetBroker();
        };

    } // namespace ipc

} // namespace gbs

#endif // __IPC_BROKER_NET_H__