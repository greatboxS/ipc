#ifndef __CLIENT_ADAPTOR_H__
#define __CLIENT_ADAPTOR_H__

#include "dbus_client_proxy.h"
#include <map>
#include <mutex>
#include <queue>
#include <sdbus-c++/sdbus-c++.h>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define DEFAULT_MESG_SIZE  (uint64_t)(1024 * 10)
#define DEFAULT_MESG_COUNT (uint64_t)1024

namespace gbs
{
    namespace ipc
    {
        namespace dbus
        {
            extern const std::string BROKER_PATH;
            extern const std::string BROKER_DEST;
            extern const std::string BROKER_NAME;

            struct DbusService {
                int32_t id;
                std::string name;
                bool connected;
                std::unique_ptr<ClientProxyInterface> proxy;
                struct manage {
                    size_t queueSize;
                    uint64_t totalData;
                } manager;
            };

            class Broker_adaptor
            {
            public:
                static constexpr const char *INTERFACE_NAME = "gbs.ipc.dbus.Broker";

            protected:
                Broker_adaptor(sdbus::IObject &object) :
                    object_(object) {
                    object_.registerMethod("connect").onInterface(INTERFACE_NAME).withInputParamNames("sender", "path").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &path) { return this->connect(sender, path); });
                    object_.registerMethod("disconnect").onInterface(INTERFACE_NAME).withInputParamNames("sender").withOutputParamNames("arg0").implementedAs([this](const std::string &sender) { return this->disconnect(sender); });
                    object_.registerMethod("connectedServices").onInterface(INTERFACE_NAME).withOutputParamNames("arg0").implementedAs([this]() { return this->connectedServices(); });
                    object_.registerMethod("connected").onInterface(INTERFACE_NAME).withInputParamNames("sender").withOutputParamNames("arg0").implementedAs([this](const std::string &sender) { return this->connected(sender); });
                    object_.registerMethod("sendMessage").onInterface(INTERFACE_NAME).withInputParamNames("sender", "receiver", "message").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &receiver, const std::vector<uint8_t> &message) {
                        return this->sendMessage(sender, receiver, message);
                    });
                    object_.registerMethod("subscribe").onInterface(INTERFACE_NAME).withInputParamNames("sender", "topic").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &topic) {
                        return this->subscribe(sender, topic);
                    });
                    object_.registerMethod("unsubscribe").onInterface(INTERFACE_NAME).withInputParamNames("sender", "topic").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &topic) {
                        return this->unsubscribe(sender, topic);
                    });
                    object_.registerMethod("publishMessage").onInterface(INTERFACE_NAME).withInputParamNames("sender", "topic", "message").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) {
                        return this->publishMessage(sender, topic, message);
                    });
                    object_.registerProperty("name").onInterface(INTERFACE_NAME).withGetter([this]() { return this->name(); });
                    object_.registerProperty("messageSize").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageSize(); });
                    object_.registerProperty("messageCount").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageCount(); });
                    object_.registerProperty("commonError").onInterface(INTERFACE_NAME).withGetter([this]() { return this->commonError(); });
                }

                ~Broker_adaptor() = default;

            protected:
                virtual int32_t connect(const std::string &sender, const std::string &path) = 0;
                virtual int32_t disconnect(const std::string &sender) = 0;
                virtual std::vector<std::string> connectedServices() = 0;
                virtual bool connected(const std::string &sender) = 0;
                virtual int32_t sendMessage(const std::string &sender, const std::string &receiver, const std::vector<uint8_t> &message) = 0;
                virtual int32_t subscribe(const std::string &sender, const std::string &topic) = 0;
                virtual int32_t unsubscribe(const std::string &sender, const std::string &topic) = 0;
                virtual int32_t publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) = 0;
                virtual std::string name() = 0;
                virtual uint64_t messageSize() = 0;
                virtual uint64_t messageCount() = 0;
                virtual int32_t commonError() = 0;

            private:
                sdbus::IObject &object_;
            };

            class ServerManagerAdaptor : public sdbus::AdaptorInterfaces<sdbus::ObjectManager_adaptor>
            {
            public:
                ServerManagerAdaptor(sdbus::IConnection &connection, std::string path = BROKER_PATH) :
                    AdaptorInterfaces(connection, std::move(path)) {
                    registerAdaptor();
                }

                ~ServerManagerAdaptor() {
                    unregisterAdaptor();
                }
            };

            class ServerAdaptor : public sdbus::AdaptorInterfaces<gbs::ipc::dbus::Broker_adaptor,
                                                                  sdbus::ManagedObject_adaptor,
                                                                  sdbus::Properties_adaptor>
            {
            public:
                explicit ServerAdaptor(sdbus::IConnection &connection, std::string path = BROKER_PATH, std::string name = BROKER_NAME,
                                       uint64_t mesgSize = DEFAULT_MESG_SIZE, uint64_t mesgCount = DEFAULT_MESG_COUNT) :
                    m_connection(connection),
                    m_path(path),
                    AdaptorInterfaces(connection, path),
                    m_name(std::move(name)),
                    m_messageSize(mesgSize),
                    m_messageCount(mesgCount) {
                    registerAdaptor();
                    emitInterfacesAddedSignal({gbs::ipc::dbus::Broker_adaptor::INTERFACE_NAME});
                }

                virtual ~ServerAdaptor() {
                    emitInterfacesRemovedSignal({gbs::ipc::dbus::Broker_adaptor::INTERFACE_NAME});
                    unregisterAdaptor();
                }

                inline bool messageQueueEmpty() {
                    std::shared_lock<std::shared_mutex> lock(m_mtx);
                    return m_mesgQueue.empty();
                }

                inline std::pair<std::string, std::string> readMessage() {
                    std::shared_lock<std::shared_mutex> lock(m_mtx);
                    return m_mesgQueue.front();
                }

                inline void popMessage() {
                    std::unique_lock<std::shared_mutex> lock(m_mtx);
                    return m_mesgQueue.pop();
                }

            protected:
                virtual void onConnectionChanged(const std::string &name, dbus::DbusService &service) = 0;

            private:
                virtual int32_t connect(const std::string &sender, const std::string &path);
                virtual int32_t disconnect(const std::string &sender);
                virtual std::vector<std::string> connectedServices();
                virtual bool connected(const std::string &sender);
                virtual int32_t sendMessage(const std::string &sender, const std::string &receiver, const std::vector<uint8_t> &message) noexcept;
                virtual int32_t subscribe(const std::string &sender, const std::string &topic) noexcept;
                virtual int32_t unsubscribe(const std::string &sender, const std::string &topic) noexcept;
                virtual int32_t publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) noexcept;
                virtual std::string name() { return m_name; }
                virtual uint64_t messageSize() { return m_messageSize; }
                virtual uint64_t messageCount() { return m_messageCount; }
                virtual int32_t commonError() { return m_commonError; }

            protected:
                sdbus::IConnection &m_connection;
                std::string m_path;
                std::shared_mutex m_mtx;
                std::queue<std::pair<std::string, std::string>> m_mesgQueue;
                std::map<std::string, DbusService> m_services;
                std::unordered_map<std::string, std::unordered_set<std::string>> m_topics;
                std::unordered_map<std::string, std::unordered_set<std::string>> m_subscribers;
                std::string m_name;
                uint64_t m_messageSize;
                uint64_t m_messageCount;
                int32_t m_commonError;
            };

        }
    }
} // namespaces

#endif // __CLIENT_ADAPTOR_H__