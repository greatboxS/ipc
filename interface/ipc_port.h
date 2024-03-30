#ifndef __IPC_PORT_H__
#define __IPC_PORT_H__

#include "ipc_type.h"
#include <memory>

namespace gbs
{
    namespace ipc
    {
        class IpcProfile;

        class IpcPort
        {
        private:
        public:
            virtual ~IpcPort() = default;
            virtual int id() const = 0;
            virtual const IpcProfile *profile() const = 0;
            virtual const std::shared_ptr<IpcMessage> read() = 0;
            virtual int write(const IpcMessage &msg) = 0;
            virtual int subscribe(std::string &topic, IpcTopicCallback callback) = 0;
            virtual int unSubscribe(std::string &topic) = 0;
            virtual int publishMessage(std::string &topic, const IpcMessage &message) = 0;
        };
    } // namespace ipc
} // namespace gbs
#endif // __IPC_PORT_H__