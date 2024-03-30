#include "ipc_proxy_shm.h"
#include "ipc_profile.h"
#include "port/shm/ipc_port_shm.h"

namespace gbs {
namespace ipc {
IpcShmProxy::IpcShmProxy() {}
IpcShmProxy::~IpcShmProxy() {}

int IpcShmProxy::connect() {
    return 0;
}
int IpcShmProxy::disconnect() {
    return 0;
}
bool IpcShmProxy::connected() {
    return 0;
}
IpcPort *IpcShmProxy::port() {
    return 0;
}
} // namespace ipc

} // namespace gbs
