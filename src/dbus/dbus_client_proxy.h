#ifndef __DBUS_CLIENT_PROXY_H__
#define __DBUS_CLIENT_PROXY_H__

#include <iostream>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <tuple>
#include <unordered_map>

namespace gbs
{
    namespace ipc
    {
        namespace dbus
        {
            class Client_proxy
            {
            protected:
                uint64_t m_sentCounter;
                uint64_t m_publishCounter;
                uint64_t m_sentErrCounter;
                uint64_t m_publishErrCounter;

            public:
                static constexpr const char *INTERFACE_NAME = "gbs.ipc.dbus.Client";

            protected:
                Client_proxy(sdbus::IProxy &proxy) :
                    proxy_(proxy),
                    m_sentCounter(0),
                    m_publishCounter(0),
                    m_sentErrCounter(0),
                    m_publishErrCounter(0) {
                }

                ~Client_proxy() = default;

                virtual void onSendMessageReply(const int32_t &ret, const sdbus::Error *error) = 0;
                virtual void onPublishMessageReply(const int32_t &ret, const sdbus::Error *error) = 0;

            public:
                sdbus::PendingAsyncCall sendMessage(const std::string &sender, const std::string &message) {
                    const std::vector<uint8_t> msg(message.begin(), message.end());
                    m_sentCounter++;
                    return sendMessage(sender, msg);
                }

                sdbus::PendingAsyncCall publishMessage(const std::string &sender, const std::string &topic, const std::string &message) {
                    const std::vector<uint8_t> msg(message.begin(), message.end());
                    m_publishCounter++;
                    return publishMessage(sender, topic, msg);
                }

                sdbus::PendingAsyncCall sendMessage(const std::string &sender, const std::vector<uint8_t> &message) {
                    m_sentCounter++;
                    return proxy_.callMethodAsync("sendMessage").onInterface(INTERFACE_NAME).withArguments(sender, message).uponReplyInvoke([this](const sdbus::Error *error, const int32_t &ret) {
                        this->onSendMessageReply(ret, error);
                    });
                }

                sdbus::PendingAsyncCall publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) {
                    m_publishCounter++;
                    return proxy_.callMethodAsync("publishMessage").onInterface(INTERFACE_NAME).withArguments(sender, topic, message).uponReplyInvoke([this](const sdbus::Error *error, const int32_t &ret) {
                        this->onPublishMessageReply(ret, error);
                    });
                }

                std::string name() {
                    return proxy_.getProperty("name").onInterface(INTERFACE_NAME);
                }

                uint64_t messageSize() {
                    return proxy_.getProperty("messageSize").onInterface(INTERFACE_NAME);
                }

                uint64_t messageCount() {
                    return proxy_.getProperty("messageCount").onInterface(INTERFACE_NAME);
                }

                int32_t commonError() {
                    return proxy_.getProperty("commonError").onInterface(INTERFACE_NAME);
                }

                uint64_t totalPackageLost() { return (m_sentErrCounter + m_publishErrCounter); }

                uint64_t totalPackageSent() { return (m_sentCounter + m_publishCounter); }

            private:
                sdbus::IProxy &proxy_;
            };

            /**
             * @class ClientProxyInterface
             * @brief
             *
             */
            class ClientProxyInterface final : public sdbus::ProxyInterfaces<gbs::ipc::dbus::Client_proxy>
            {
            public:
                explicit ClientProxyInterface(sdbus::IConnection &connection, std::string destination, std::string path) :
                    ProxyInterfaces(connection, std::move(destination), std::move(path)) {
                    registerProxy();
                }

                virtual ~ClientProxyInterface() {
                    unregisterProxy();
                }

                virtual void onSendMessageReply(const int32_t &ret, const sdbus::Error *error) {
                    if (error && error->isValid()) {
                        m_sentErrCounter++;
                    }
                }
                virtual void onPublishMessageReply(const int32_t &ret, const sdbus::Error *error) {
                    if (error && error->isValid()) {
                        m_publishErrCounter++;
                    }
                }

            private:
            };

            /**
             * @class ClientManagerProxy
             * @brief
             *
             */
            class ClientManagerProxy final : public sdbus::ProxyInterfaces<sdbus::ObjectManager_proxy>
            {
            public:
                struct ClientInterface {
                    ClientProxyInterface *interface;
                    std::string objectPath;
                    std::string name;
                    uint64_t mesgSize;
                    uint64_t mesgCount;
                };

                explicit ClientManagerProxy(sdbus::IConnection &connection, const std::string &destination, std::string path) noexcept :
                    ProxyInterfaces(connection, destination, std::move(path)) {
                    registerProxy();
                }

                ~ClientManagerProxy() noexcept {
                    unregisterProxy();
                }

                void handleExistingObjects() noexcept;

            private:
                void onInterfacesAdded(const sdbus::ObjectPath &objectPath, const std::map<std::string, std::map<std::string, sdbus::Variant>> &interfacesAndProperties) noexcept override;
                void onInterfacesRemoved(const sdbus::ObjectPath &objectPath, const std::vector<std::string> &interfaces) noexcept override;
            };
        }
    } // namespaces
}
#endif // __DBUS_CLIENT_PROXY_H__