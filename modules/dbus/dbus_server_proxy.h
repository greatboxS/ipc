#ifndef __DBUS_PROXY_H__
#define __DBUS_PROXY_H__

#include <iostream>
#include <sdbus-c++/sdbus-c++.h>
#include <string>
#include <tuple>
#include <unordered_map>

namespace gbs {
namespace ipc {
namespace dbus {
extern const std::string BROKER_PATH;
extern const std::string BROKER_DEST;

class Broker_proxy {
public:
    static constexpr const char *INTERFACE_NAME = "gbs.ipc.dbus.Broker";

protected:
    Broker_proxy(sdbus::IProxy &proxy) :
        proxy_(proxy) {
    }

    ~Broker_proxy() = default;

public:
    int32_t connect(const std::string &sender, const std::string &path) {
        int32_t result;
        proxy_.callMethod("connect").onInterface(INTERFACE_NAME).withArguments(sender, path).storeResultsTo(result);
        return result;
    }

    int32_t disconnect(const std::string &sender) {
        int32_t result;
        proxy_.callMethod("disconnect").onInterface(INTERFACE_NAME).withArguments(sender).storeResultsTo(result);
        return result;
    }

    std::vector<std::string> connectedServices() {
        std::vector<std::string> result;
        proxy_.callMethod("connectedServices").onInterface(INTERFACE_NAME).storeResultsTo(result);
        return result;
    }

    bool connected(const std::string &sender) {
        bool result;
        proxy_.callMethod("connected").onInterface(INTERFACE_NAME).withArguments(sender).storeResultsTo(result);
        return result;
    }

    int32_t sendMessage(const std::string &sender, const std::string &receiver, const std::vector<uint8_t> &message) {
        int32_t result;
        proxy_.callMethod("sendMessage").onInterface(INTERFACE_NAME).withArguments(sender, receiver, message).storeResultsTo(result);
        return result;
    }

    int32_t sendMessage(const std::string &sender, const std::string &receiver, const std::string &message) {
        const std::vector<uint8_t> msg(message.begin(), message.end());
        return sendMessage(sender, receiver, msg);
    }

    int32_t subscribe(const std::string &sender, const std::string &topic) {
        int32_t result;
        proxy_.callMethod("subscribe").onInterface(INTERFACE_NAME).withArguments(sender, topic).storeResultsTo(result);
        return result;
    }

    int32_t unsubscribe(const std::string &sender, const std::string &topic) {
        int32_t result;
        proxy_.callMethod("unsubscribe").onInterface(INTERFACE_NAME).withArguments(sender, topic).storeResultsTo(result);
        return result;
    }

    int32_t publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) {
        int32_t result;
        proxy_.callMethod("publishMessage").onInterface(INTERFACE_NAME).withArguments(sender, topic, message).storeResultsTo(result);
        return result;
    }

    int32_t publishMessage(const std::string &sender, const std::string &receiver, const std::string &message) {
        const std::vector<uint8_t> msg(message.begin(), message.end());
        return publishMessage(sender, receiver, msg);
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

private:
    sdbus::IProxy &proxy_;
};

/**
 * @class ServerManagerProxy
 * @brief
 *
 */
class ServerProxyInterface final : public sdbus::ProxyInterfaces<gbs::ipc::dbus::Broker_proxy> {
public:
    explicit ServerProxyInterface(sdbus::IConnection &connection, std::string dest = BROKER_DEST, std::string path = BROKER_PATH) :
        ProxyInterfaces(connection, std::move(dest), std::move(path)) {
        registerProxy();
    }

    virtual ~ServerProxyInterface() {
        unregisterProxy();
    }

private:
    std::map<std::string, int32_t> m_errorMap;
};

/**
 * @class ServerManagerProxy
 * @brief
 *
 */
class ServerManagerProxy final : public sdbus::ProxyInterfaces<sdbus::ObjectManager_proxy> {
public:
    struct ClientInterface {
        ServerManagerProxy *interface;
        std::string objectPath;
        std::string name;
        uint64_t mesgSize;
        uint64_t mesgCount;
    };

    explicit ServerManagerProxy(sdbus::IConnection &connection, const std::string &dest = BROKER_DEST, std::string path = BROKER_PATH) noexcept :
        ProxyInterfaces(connection, dest, std::move(path)),
        m_connection(connection), m_destination(dest) {
        registerProxy();
    }

    ~ServerManagerProxy() noexcept {
        unregisterProxy();
    }

    void handleExistingObjects() noexcept;

    const std::unordered_map<std::string, ClientInterface> &getClientProxyInterface() const { return m_clients; }

private:
    void onInterfacesAdded(const sdbus::ObjectPath &objectPath, const std::map<std::string, std::map<std::string, sdbus::Variant>> &interfacesAndProperties) noexcept override;
    void onInterfacesRemoved(const sdbus::ObjectPath &objectPath, const std::vector<std::string> &interfaces) noexcept override;

    sdbus::IConnection &m_connection;
    std::string m_destination;
    std::unordered_map<std::string, ClientInterface> m_clients;
};
} // namespace dbus
} // namespace ipc
} // namespace gbs
#endif // __CLIENT_PROXY_H__