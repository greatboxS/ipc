#include "ipc_proxy_net.h"
#include "ipc_profile.h"
#include "port/net/ipc_port_net.h"

namespace gbs
{
    namespace ipc
    {
        IpcNetProxy::IpcNetProxy() {}
        IpcNetProxy::~IpcNetProxy() {}

        int IpcNetProxy::connect() {
            return 0;
        }
        int IpcNetProxy::disconnect() {
            return 0;
        }
        bool IpcNetProxy::connected() {
            return 0;
        }
        IpcPort *IpcNetProxy::port() {
            return 0;
        }
    } // namespace ipc

} // namespace gbs
