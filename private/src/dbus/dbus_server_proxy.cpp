#include "dbus_server_proxy.h"

namespace gbs {
namespace ipc {
namespace dbus {
const std::string BROKER_PATH = "/gbs/ipc/dbus/Broker";
const std::string BROKER_DEST = "gbs.ipc.dbus.Broker";
const std::string BROKER_NAME = "gbs.ipc.dbus.Broker.Service";

void ServerManagerProxy::handleExistingObjects() noexcept {
    std::map<sdbus::ObjectPath, std::map<std::string, std::map<std::string, sdbus::Variant>>> objectsInterfacesAndProperties;
    objectsInterfacesAndProperties = GetManagedObjects();
    for (const auto &[object, interfacesAndProperties] : objectsInterfacesAndProperties) {
        onInterfacesAdded(object, interfacesAndProperties);
    }
}

void ServerManagerProxy::onInterfacesAdded(const sdbus::ObjectPath &objectPath, const std::map<std::string, std::map<std::string, sdbus::Variant>> &interfacesAndProperties) noexcept {
    std::cout << objectPath << " added:\t";
    for (const auto &[interface, _] : interfacesAndProperties) {
        std::cout << interface << " ";
    }
    std::cout << std::endl;

    // Parse and print some more info
    auto interface = interfacesAndProperties.find(Broker_proxy::INTERFACE_NAME);
    if (interface == interfacesAndProperties.end()) return;

    const auto &properties = interface->second;
    ServerManagerProxy *proxy = new ServerManagerProxy(m_connection, m_destination, objectPath);
    std::string name = "";
    uint64_t msgSize = 0;
    uint64_t msgCount = 0;

    if (properties.find("name") != properties.end())
        name = properties.at("name").get<std::string>();

    if (properties.find("messageSize") != properties.end())
        msgSize = properties.at("messageSize").get<uint64_t>();

    if (properties.find("messageSize") != properties.end())
        msgCount = properties.at("messageCount").get<uint64_t>();

    m_clients[objectPath] = {proxy, objectPath, name, msgSize, msgCount};
}

void ServerManagerProxy::onInterfacesRemoved(const sdbus::ObjectPath &objectPath, const std::vector<std::string> &interfaces) noexcept {
    std::cout << objectPath << " removed:\t";
    for (const auto &interface : interfaces) {
        std::cout << interface << " ";
    }
    std::cout << std::endl;

    auto interface = m_clients.find(objectPath);
    if (interface == m_clients.end()) return;

    delete m_clients[objectPath].interface;
    m_clients.erase(interface);
}
} // namespace dbus
} // namespace ipc
} // namespace gbs