#ifndef DBUS_SERVICE_H
#define DBUS_SERVICE_H

#include <string>

namespace ipc::core {
class dbus_service {
public:
    enum bus_type {
        section_bus,
        system_bus,
    };

    dbus_service(bus_type bus, const std::string &interface);

    static bool register_service(const std::string &service);
    static bool is_registered(const std::string &service, bus_type bus);

    
};
} // namespace ipc::core

#endif // DBUS_SERVICE_H