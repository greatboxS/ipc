#include "ipc_broker_shm.h"
#include "ipc_profile.h"
namespace gbs {
namespace ipc {
IpcShmBroker::IpcShmBroker(const IpcProfile *profile) {}

IpcShmBroker::~IpcShmBroker() {
}

int IpcShmBroker::onBrokerStart() {
    return 0;
}
int IpcShmBroker::onBrokerStop() {
    return 0;
}
int IpcShmBroker::onBrokerSend(const IpcProfile *profile, const std::string &message) {
    return 0;
}
int IpcShmBroker::onBrokerSubscribe(const IpcProfile *profile, const std::string &topic) {
    return 0;
}
int IpcShmBroker::onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic) {
    return 0;
}
int IpcShmBroker::onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) {
    return 0;
}

} // namespace ipc

} // namespace gbs
