#include "ipc_port_net.h"
#include "ipc_profile.h"
namespace gbs {
namespace ipc {
static int32_t sId = 0;
IpcNetPort::IpcNetPort(const IpcProfile *profile, const std::string &host, uint16_t port) :
    m_id(++sId),
    m_profile(profile),
    m_host(host), m_port(port) {
}
IpcNetPort::~IpcNetPort() {
}
const std::shared_ptr<IpcMessage> IpcNetPort::read() {
    return NULL;
}
int IpcNetPort::write(const IpcMessage &msg) {
    return 0;
}
int IpcNetPort::subscribe(std::string &topic, IpcTopicCallback callback) {
    return 0;
}
int IpcNetPort::unSubscribe(std::string &topic) {
    return 0;
}
int IpcNetPort::publishMessage(std::string &topic, const IpcMessage &message) {
    return 0;
}
} // namespace ipc
} // namespace gbs