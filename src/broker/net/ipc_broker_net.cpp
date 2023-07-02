#include "ipc_broker_net.h"
#include "ipc_profile.h"
namespace gbs
{
    namespace ipc
    {
        IpcNetBroker::IpcNetBroker(const IpcProfile *profile) {}

        IpcNetBroker::~IpcNetBroker() {
        }

        int IpcNetBroker::onBrokerStart() {
            return 0;
        }
        int IpcNetBroker::onBrokerStop() {
            return 0;
        }
        int IpcNetBroker::onBrokerSend(const IpcProfile *profile, const std::string &message) {
            return 0;
        }
        int IpcNetBroker::onBrokerSubscribe(const IpcProfile *profile, const std::string &topic) {
            return 0;
        }
        int IpcNetBroker::onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic) {
            return 0;
        }
        int IpcNetBroker::onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) {
            return 0;
        }

    } // namespace ipc

} // namespace gbs
