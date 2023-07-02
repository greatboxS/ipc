#include "ipc_proxy_dbus.h"
#include "ipc_profile.h"
#include "dbus/dbus_client_adaptor.h"
#include "dbus/dbus_server_proxy.h"
#include "port/dbus/ipc_port_dbus.h"

namespace gbs
{
    namespace ipc
    {
        IpcDbusProxy::IpcDbusProxy(const IpcProfile *profile,
                                   std::unique_ptr<sdbus::IConnection> connection,
                                   const std::string &path, const std::string &dest) :
            m_profile(profile),
            m_path(path),
            m_connection(std::move(connection)) {
            std::string name = profile->name();

            try {
                m_connection->requestName(profile->name());
            }
            catch (const sdbus::Error &e) {
                IPC_ERROR("Request service name %s failed, %s\n", profile->name().c_str(), e.what());
            }

            m_serverProxyInterface = std::make_unique<dbus::ServerProxyInterface>(*m_connection);
            m_port = std::make_unique<IpcDbusPort>(profile, *m_serverProxyInterface, *m_connection, m_path, name);

            try {
                m_connection->enterEventLoopAsync();
            }
            catch (const sdbus::Error &e) {
                IPC_ERROR("Enter event loop failed, %s\n", e.what());
            }
        }

        IpcDbusProxy::~IpcDbusProxy() {
            m_connection->leaveEventLoop();
        }

        int IpcDbusProxy::connect() try {
            m_serverProxyInterface->connect(m_profile->name(), m_path);
            return 0;
        }
        IPC_HANDLE(return -1);

        int IpcDbusProxy::disconnect() {
            m_serverProxyInterface->disconnect(m_profile->name());
            return 0;
        }

        bool IpcDbusProxy::connected() {
            return m_serverProxyInterface->connected(m_profile->name());
        }

        IpcPort *IpcDbusProxy::port() { return reinterpret_cast<IpcPort *>(m_port.get()); }

    } // namespace ipc

} // namespace gbs
