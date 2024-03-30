#ifndef __DBUS_CLIENT_ADAPTOR_H__
#define __DBUS_CLIENT_ADAPTOR_H__

#include <mutex>
#include <queue>
#include <sdbus-c++/sdbus-c++.h>
#include <shared_mutex>
#include <string>
#include <tuple>

#define DEFAULT_MESG_SIZE  (uint64_t)(1024 * 10)
#define DEFAULT_MESG_COUNT (uint64_t)1024

namespace gbs
{
    namespace ipc
    {
        namespace dbus
        {
            class Client_adaptor
            {
            public:
                static constexpr const char *INTERFACE_NAME = "gbs.ipc.dbus.Client";

            protected:
                Client_adaptor(sdbus::IObject &object) :
                    object_(object) {
                    object_.registerMethod("sendMessage").onInterface(INTERFACE_NAME).withInputParamNames("sender", "message").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::vector<uint8_t> &message) {
                        return this->sendMessage(sender, message);
                    });
                    object_.registerMethod("publishMessage").onInterface(INTERFACE_NAME).withInputParamNames("sender", "topic", "message").withOutputParamNames("arg0").implementedAs([this](const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) {
                        return this->publishMessage(sender, topic, message);
                    });
                    object_.registerProperty("name").onInterface(INTERFACE_NAME).withGetter([this]() { return this->name(); });
                    object_.registerProperty("messageSize").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageSize(); });
                    object_.registerProperty("messageCount").onInterface(INTERFACE_NAME).withGetter([this]() { return this->messageCount(); });
                    object_.registerProperty("commonError").onInterface(INTERFACE_NAME).withGetter([this]() { return this->commonError(); });
                }

                ~Client_adaptor() = default;

                virtual std::string name() = 0;
                virtual uint64_t messageSize() = 0;
                virtual uint64_t messageCount() = 0;
                virtual int32_t commonError() = 0;
                virtual int32_t sendMessage(const std::string &sender, const std::vector<uint8_t> &message) = 0;
                virtual int32_t publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) = 0;

            private:
                sdbus::IObject &object_;
            };

            class ClientManagerAdaptor : public sdbus::AdaptorInterfaces<sdbus::ObjectManager_adaptor>
            {
            public:
                ClientManagerAdaptor(sdbus::IConnection &connection, std::string path) :
                    AdaptorInterfaces(connection, std::move(path)) {
                    registerAdaptor();
                }

                ~ClientManagerAdaptor() {
                    unregisterAdaptor();
                }
            };

            class ClientAdaptor : public sdbus::AdaptorInterfaces<gbs::ipc::dbus::Client_adaptor,
                                                                  sdbus::ManagedObject_adaptor,
                                                                  sdbus::Properties_adaptor>
            {
            public:
                explicit ClientAdaptor(sdbus::IConnection &connection, std::string path, std::string name,
                                       int32_t mesgSize = DEFAULT_MESG_SIZE, int32_t mesgCount = DEFAULT_MESG_COUNT) :
                    AdaptorInterfaces(connection, path),
                    m_name(std::move(name)),
                    m_messageSize(mesgSize),
                    m_messageCount(mesgCount) {
                    registerAdaptor();
                    emitInterfacesAddedSignal({gbs::ipc::dbus::Client_adaptor::INTERFACE_NAME});
                }

                virtual ~ClientAdaptor() {
                    emitInterfacesRemovedSignal({gbs::ipc::dbus::Client_adaptor::INTERFACE_NAME});
                    unregisterAdaptor();
                }

            private:
                virtual std::string name() { return m_name; }
                virtual uint64_t messageSize() { return m_messageSize; }
                virtual uint64_t messageCount() { return m_messageCount; }
                virtual int32_t commonError() { return m_commonError; }

            protected:
                std::shared_mutex m_mtx;
                std::string m_name;
                uint64_t m_messageSize;
                uint64_t m_messageCount;
                int32_t m_commonError;
            };
        } // namespace dbus
    }     // namespace ipc
} // namespace gbs

#endif // __DBUS_CLIENT_ADAPTOR_H__