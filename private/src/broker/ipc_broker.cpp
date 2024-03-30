#include "ipc_broker.h"
#include "../../include/ipc_profile.h"
#include <mutex>

namespace gbs
{
    namespace ipc
    {
        void IpcBrokerBase::brokerMainLoop() {
            {
                std::shared_lock<std::shared_mutex> lock(m_mtx);
                int result = onBrokerStart();
            }

            while (m_running) {
                onBrokerRunning();
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            onBrokerStop();
        }

        IpcBrokerBase::IpcBrokerBase() :
            m_running(false) {
        }

        int IpcBrokerBase::start() {
            m_running = true;
            {
                std::unique_lock<std::shared_mutex> lock(m_mtx);
                std::thread mainLoopThread(&IpcBrokerBase::brokerMainLoop, this);
                mainLoopThread.detach();
            }
            return 0;
        }

        int IpcBrokerBase::stop() {
            m_running = false;
            return 0;
        }

        int IpcBrokerBase::subscribe(const IpcProfile *profile, const std::string &topic, IpcTopicCallback callback) {
            int result = onBrokerSubscribe(profile, topic);
            if (result == 0) {
            }
            return result;
        }

        int IpcBrokerBase::unsubscribe(const IpcProfile *profile, const std::string &topic) {
            int result = onBrokerUnSubscribe(profile, topic);
            if (result == 0) {
            }
            return result;
        }

        int IpcBrokerBase::publicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) {
            int result = onBrokerPublicMessage(profile, topic, message);
            if (result == 0) {
            }
            return result;
        }

        int IpcBrokerBase::sendMessage(const IpcProfile *profile, const std::string &message) {
            return onBrokerSend(profile, message);
        }
    } // namespace ipc
} // namespace gbs
