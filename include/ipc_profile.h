#ifndef __IPC_PROFILE_H__
#define __IPC_PROFILE_H__

#include "ipc_def.h"

namespace gbs
{
    namespace ipc
    {
        typedef int32_t ProfileId;
        typedef int32_t GroupId;
        typedef int32_t AppId;

        enum class IpcType {
            SHM = 0,
            SOCKET,
            DBUS,
        };

        class IpcProfile
        {
        private:
        public:
            virtual ~IpcProfile() = default;
            virtual ProfileId id() const = 0;
            virtual const std::string &name() const = 0;
            virtual GroupId groupdId() const = 0;
            virtual AppId appId() const = 0;

            static int32_t getServiceId(const std::string &name);
            static const std::string getServiceName(int32_t id);
        };
    } // namespace ipc
} // namespace gbs

#endif // __IPC_PROFILE_H__