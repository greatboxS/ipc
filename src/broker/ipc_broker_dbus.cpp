#include "ipc_broker_dbus.h"
#include "dbus/dbus_client_proxy.h"
#include "ipc_profile.h"
namespace gbs
{
    namespace ipc
    {
        IpcDbusBroker::IpcDbusBroker(const IpcProfile *profile, sdbus::IConnection &connection, std::string path, std::string name,
                                     uint64_t mesgSize, uint64_t mesgCount) :
            dbus::ServerManagerAdaptor(connection),
            dbus::ServerAdaptor(connection, path, name, mesgSize, mesgCount),
            m_profile(profile) {
        }

        IpcDbusBroker::~IpcDbusBroker() {
        }

        int IpcDbusBroker::onBrokerStart() {
            m_connection.enterEventLoopAsync();
            return 0;
        }
        int IpcDbusBroker::onBrokerStop() {
            m_connection.leaveEventLoop();
            return 0;
        }
        int IpcDbusBroker::onBrokerSend(const IpcProfile *profile, const std::string &message) {
            return 0;
        }
        int IpcDbusBroker::onBrokerSubscribe(const IpcProfile *profile, const std::string &topic) {
            return 0;
        }
        int IpcDbusBroker::onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic) {
            return 0;
        }
        int IpcDbusBroker::onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) {
            return 0;
        }

        void IpcDbusBroker::onConnectionChanged(const std::string &name, dbus::DbusService &service) {
            IPC_INFO("onConnectionChanged\n");
        }

    } // namespace ipc

} // namespace gbs
