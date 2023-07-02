#include "../include/ipc_args.h"
#include "../include/ipc_message.h"
#include "../include/ipc_port.h"
#ifndef _MSC_VER
#include "../src/proxy/dbus/ipc_proxy_dbus.h"
#endif
#include "ipc_profile.h"
#include <chrono>
#include <iostream>
#include <thread>

namespace gbs
{
    namespace ipc
    {
        class DBusProfile : public IpcProfile
        {
            ProfileId _id;
            std::string _name;
            GroupId _groupId;
            AppId _appId;

        public:
            virtual ~DBusProfile() = default;
            DBusProfile(const std::string &n, GroupId gid, AppId aid) :
                _id(1), _name(n), _groupId(gid), _appId(aid) {
            }
            virtual ProfileId id() const { return _id; }
            virtual const std::string &name() const { return _name; }
            virtual GroupId groupdId() const { return _groupId; }
            virtual AppId appId() const { return _appId; }
        };
    } // namespace ipc
} // namespace gbs

int main(int argc, char const *argv[]) noexcept {

    auto *message = new gbs::ipc::IpcMessage("gbs.ipc.dbus.Service.Service1", "gbs.ipc.dbus.Service.Service1", "");
    gbs::ipc::IpcArgs<int, const char *, std::string> args;
    args << 1 << "dddddddddddddddfefe" << std::string("parsing data from string and return it 2ssssssssssss");
    message->setArgs(args);

#ifndef _MSC_VER
    auto connection = sdbus::createSessionBusConnection();

    auto profile = new gbs::ipc::DBusProfile("gbs.ipc.dbus.Service.Service1", 1, 1);
    gbs::ipc::IpcDbusProxy proxy(profile, std::move(connection), "/Service/Service1", "gbs.ipc.dbus.Broker");
    auto port = proxy.port();
    proxy.connect();

    while (1) {

        port->write(*message);
        auto msg = port->read();
        if (msg.get()) {

            gbs::ipc::IpcArgs<int, const char *, std::string> args(msg->content());
            std::cout << "(";
            std::apply([](const auto &...values) {
                ((std::cout << values << " "), ...);
            },
                       args.data().get());
            std::cout << ")" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
#endif
    return 0;
}