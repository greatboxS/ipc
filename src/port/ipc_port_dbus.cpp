#include "ipc_port_dbus.h"
#include "../../include/ipc_message.h"
#include "../../include/ipc_profile.h"
#include "../dbus/dbus_server_proxy.h"
#include "ipc_utility.h"

namespace gbs
{
    namespace ipc
    {
        static int sId = 0;
        IpcDbusPort::IpcDbusPort(const IpcProfile *profile,
                                 dbus::ServerProxyInterface &proxy,
                                 sdbus::IConnection &connection, std::string path, std::string name) :
            m_profile(profile),
            m_id(++sId),
            m_serverProxyInterface(proxy),
            dbus::ClientManagerAdaptor(connection, path),
            gbs::ipc::dbus::ClientAdaptor(connection, path, name) {
        }

        const std::shared_ptr<IpcMessage> IpcDbusPort::read() try {
            std::unique_lock<std::shared_mutex> locker(m_mtx);
            if (m_messageQueue.empty()) return {};

            auto mesg = m_messageQueue.front();
            m_messageQueue.pop();
            return std::shared_ptr<IpcMessage>(mesg);
        }
        IPC_HANDLE(return {});

        int IpcDbusPort::write(const IpcMessage &msg) {
            m_serverProxyInterface.sendMessage(m_profile->name(), msg.receiver(), msg.content());
            return 0;
        }

        int IpcDbusPort::subscribe(std::string &topic, IpcTopicCallback callback) {
            if (m_serverProxyInterface.subscribe(m_profile->name(), topic) != 0) return -1;
            if (m_subscribers.find(topic) == m_subscribers.end()) {
                m_subscribers[topic] = {};
            }
            for (auto &i : m_subscribers[topic]) {
                if (utility::getAddress(i) == utility::getAddress(callback)) return 0;
            }
            m_subscribers[topic].push_back(callback);
            return 0;
        }

        int IpcDbusPort::unSubscribe(std::string &topic) {
            if (m_serverProxyInterface.unsubscribe(m_profile->name(), topic) != 0) return -1;
            auto t = m_subscribers.find(topic);
            if (t == m_subscribers.end()) return -1;

            m_subscribers.erase(t);
            return 0;
        }

        int IpcDbusPort::publishMessage(std::string &topic, const IpcMessage &message) {
            m_serverProxyInterface.publishMessage(m_profile->name(), topic, message.content());
            return 0;
        }

        /**
         * @fn sendMessage
         * @brief This function used to handle the received message through dbus interface
         *
         * @param sender    sender service
         * @param receiver  receiver service
         * @param message   data
         * @return int32_t
         */
        int32_t IpcDbusPort::sendMessage(const std::string &sender, const std::vector<uint8_t> &message) {
            IPC_INFO("Received message from %s, message size: %d\n", sender.c_str(), message.size());
            std::unique_lock<std::shared_mutex> locker(m_mtx);

            m_messageQueue.push(new IpcMessage(message));
            return (int32_t)m_messageQueue.size();
        }

        /**
         * @fn publishMessage
         * @brief This function is used to receive the published message
         *
         * @param sender    sender service
         * @param topic     topic name
         * @param message   message
         * @return int32_t
         */
        int32_t IpcDbusPort::publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) {
            IPC_INFO("Received a published message from %s, message size: %d\n", sender.c_str(), message.size());
            std::unique_lock<std::shared_mutex> locker(m_mtx);

            auto sb = m_subscribers.find(topic);

            if (sb == m_subscribers.end()) return -1;

            auto msg = std::make_shared<IpcMessage>(message);
            for (auto &cb : sb->second) {
                if (cb) {
                    cb(sender, topic, msg);
                }
            }
            return 0;
        }

    } // namespace ipc

} // namespace gbs
