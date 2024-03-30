#ifndef __IPC_PROXY_DBUS_H__
#define __IPC_PROXY_DBUS_H__

#include "ipc_proxy.h"
#include <functional>
#include <iostream>
#include <sdbus-c++/sdbus-c++.h>

namespace gbs {
namespace ipc {
namespace dbus {
class ServerProxyInterface;
} // namespace dbus

class IpcDbusPort;

class IpcDbusProxy : public IpcProxy {
private:
    std::string m_path;
    const IpcProfile *m_profile;
    std::unique_ptr<sdbus::IConnection> m_connection;
    std::unique_ptr<gbs::ipc::dbus::ServerProxyInterface> m_serverProxyInterface;
    std::unique_ptr<IpcDbusPort> m_port;

public:
    explicit IpcDbusProxy(const IpcProfile *profile,
                          std::unique_ptr<sdbus::IConnection> connection,
                          const std::string &path, const std::string &dest);
    virtual ~IpcDbusProxy();

    virtual int connect();
    virtual int disconnect();
    virtual bool connected();
    virtual IpcPort *port();
};
} // namespace ipc
} // namespace gbs

#endif // __IPC_PROXY_DBUS_H__