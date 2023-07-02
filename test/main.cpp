#include "../include/ipc_args.h"
#include "../include/ipc_message.h"
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

template <typename... Args>
static void printMessage(const gbs::ipc::IpcIMessage *msg) {
    gbs::ipc::IpcArgs<Args...> args(msg->content().c_str(), msg->content().size());
    std::cout << "(";
    std::apply([](const auto &...values) {
        ((std::cout << values << " "), ...);
    },
               args.data().get());
    std::cout << ")" << std::endl;
}

int main(int argc, char const *argv[]) noexcept {

    auto *message = new gbs::ipc::IpcMessage(1, 1, 1);
    gbs::ipc::IpcArgs<int, const char *, std::string> args;
    args << 1 << "parsing data from string and return it" << std::string("ddddddddddddddddddddddd");

    args << 1 << "hello world" << std::string("parsing data from string and return it 2");
    args << 1 << "dddddddddddddddfefe" << std::string("parsing data from string and return it 2ssssssssssss");
    message->setArgs(args);
    printMessage<int, const char *, std::string>(message);

    return 0;
}