#include "dbus_server_adaptor.h"
#include "ipc_def.h"

namespace gbs
{
    namespace ipc
    {
        namespace dbus
        {
            static int32_t sId = 0;
            int32_t ServerAdaptor::connect(const std::string &sender, const std::string &path) {
                IPC_INFO("New connection requested from %s, path %s\n", sender.c_str(), path.c_str());
                std::unique_lock<std::shared_mutex> locker(m_mtx);
                auto sv = m_services.find(sender);
                if (sv == m_services.end()) {
                    m_services[sender] = {++sId,
                                          sender,
                                          true,
                                          std::move(std::make_unique<ClientProxyInterface>(m_connection, sender, path)),
                                          {}};
                    onConnectionChanged(sender, m_services[sender]);
                    return 0;
                }
                sv->second.connected = true;
                onConnectionChanged(sender, sv->second);
                return 0;
            }

            int32_t ServerAdaptor::disconnect(const std::string &sender) {
                IPC_INFO("New disconnection requested from %s\n", sender.c_str());
                std::unique_lock<std::shared_mutex> locker(m_mtx);
                auto sv = m_services.find(sender);
                if (sv == m_services.end()) { return -1; }
                sv->second.connected = false;
                onConnectionChanged(sender, sv->second);
                return 0;
            }

            std::vector<std::string> ServerAdaptor::connectedServices() {
                std::vector<std::string> services;
                std::shared_lock<std::shared_mutex> locker(m_mtx);
                for (auto &sv : m_services) {
                    if (!sv.second.connected) continue;
                    services.push_back(sv.first);
                }
                return services;
            }

            bool ServerAdaptor::connected(const std::string &sender) {
                std::shared_lock<std::shared_mutex> locker(m_mtx);
                auto sv = m_services.find(sender);
                if (sv == m_services.end()) return false;
                return sv->second.connected;
            }

            int32_t ServerAdaptor::sendMessage(const std::string &sender, const std::string &receiver, const std::vector<uint8_t> &message) noexcept {
                IPC_INFO("Received a message sent from %s, to %s, data size = %d\n", sender.c_str(), receiver.c_str(), message.size());
                std::unique_lock<std::shared_mutex> lock(m_mtx);

                auto sv = m_services.find(receiver);
                if (sv == m_services.end()) return -1;
                if (!sv->second.connected) return -1;

                sv->second.proxy->sendMessage(sender, message);
                return 0;
            }

            int32_t ServerAdaptor::subscribe(const std::string &sender, const std::string &topic) noexcept {
                IPC_INFO("Received a subscribe message from %s, to topic %s\n", sender.c_str(), topic.c_str());
                std::unique_lock<std::shared_mutex> lock(m_mtx);
                auto t = m_topics.find(topic);
                auto s = m_subscribers.find(sender);

                if (t == m_topics.end()) {
                    m_topics[topic] = {};
                }
                m_topics[topic].insert(sender);

                if (s == m_subscribers.end()) {
                    m_subscribers[sender] = {};
                }
                m_subscribers[sender].insert(topic);
                return 0;
            }

            int32_t ServerAdaptor::unsubscribe(const std::string &sender, const std::string &topic) noexcept {
                IPC_INFO("Received a un-subscribe message from %s, to topic %s\n", sender.c_str(), topic.c_str());
                std::unique_lock<std::shared_mutex> lock(m_mtx);
                auto t = m_topics.find(topic);
                auto s = m_subscribers.find(sender);

                if (t != m_topics.end()) {
                    auto fs = m_topics[topic].find(sender);
                    if (fs != m_topics[topic].end())
                        m_topics[topic].erase(fs);
                }

                if (s != m_subscribers.end()) {
                    auto ft = m_subscribers[sender].find(topic);
                    if (ft != m_subscribers[sender].end())
                        m_subscribers[sender].erase(ft);
                }
                return 0;
            }

            int32_t ServerAdaptor::publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) noexcept {
                IPC_INFO("Received a un-subscribe message from %s, to topic %s\n", sender.c_str(), topic.c_str());
                std::unique_lock<std::shared_mutex> lock(m_mtx);

                auto t = m_topics.find(topic);
                auto s = m_subscribers.find(sender);

                if (t == m_topics.end()) return -1;

                for (auto i = t->second.begin(); i != t->second.end(); i++) {
                    if (m_services.find(*i) == m_services.end()) continue;

                    auto proxy = m_services[*i].proxy.get();
                    proxy->publishMessage(sender, topic, message);
                }

                return 0;
            }
        }

    } // namespace ipc
} // namespace gbs
