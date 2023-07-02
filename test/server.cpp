#include "../include/ipc_args.h"
#include "../include/ipc_message.h"
#include "../src/broker/ipc_broker_dbus.h"
#include "ipc_profile.h"
#include <iostream>

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

    auto connection = sdbus::createSessionBusConnection("gbs.ipc.dbus.Broker");

    gbs::ipc::IpcProfile *profile = new gbs::ipc::DBusProfile("gbs.ipc.dbus.Broker.Broker1", 1, 1);
    gbs::ipc::IpcDbusBroker broker(profile, *connection, "/gbs/ipc/dbus/Broker", "gbs.ipc.dbus.Broker");
    broker.start();
    while (1)
        ;

    return 0;
}