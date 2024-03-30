#ifndef __IPC_BROKER_DBUS_H__
#define __IPC_BROKER_DBUS_H__

#include "dbus/dbus_server_adaptor.h"
#include "ipc_broker.h"

namespace gbs
{
    namespace ipc
    {
        class IpcProfile;

        class IpcDbusBroker : private dbus::ServerManagerAdaptor,
                              public dbus::ServerAdaptor,
                              public IpcBrokerBase
        {
            const IpcProfile *m_profile;

        private:
            virtual int onBrokerStart();
            virtual int onBrokerStop();
            virtual int onBrokerSend(const IpcProfile *profile, const std::string &message);
            virtual int onBrokerSubscribe(const IpcProfile *profile, const std::string &topic);
            virtual int onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic);
            virtual int onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message);

            virtual void onConnectionChanged(const std::string &name, dbus::DbusService &service);

        public:
            explicit IpcDbusBroker(const IpcProfile *profile, sdbus::IConnection &connection, std::string path, std::string name,
                                   uint64_t mesgSize = DEFAULT_MESG_SIZE, uint64_t mesgCount = DEFAULT_MESG_COUNT);
            virtual ~IpcDbusBroker();
        };

    } // namespace ipc

} // namespace gbs

#endif // __IPC_BROKER_DBUS_H__