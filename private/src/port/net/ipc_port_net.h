#ifndef __IPC_PORT_NET_H__
#define __IPC_PORT_NET_H__

#include "ipc_port.h"
#include "ipc_type.h"
#include "os/osac/CSocket.h"

namespace gbs
{
    namespace ipc
    {
        class IpcNetPort : public IpcPort
        {
            int32_t m_id;
            const IpcProfile *m_profile;
            std::string m_host;
            uint16_t m_port;

        public:
            explicit IpcNetPort(const IpcProfile *profile, const std::string &host, uint16_t port);
            virtual ~IpcNetPort();
            virtual int id() const { return m_id; }
            virtual const IpcProfile *profile() const { return m_profile; }
            virtual const std::shared_ptr<IpcMessage> read();
            virtual int write(const IpcMessage &msg);
            virtual int subscribe(std::string &topic, IpcTopicCallback callback);
            virtual int unSubscribe(std::string &topic);
            virtual int publishMessage(std::string &topic, const IpcMessage &message);
        };
    } // namespace ipc

} // namespace gbs

#endif // __IPC_PORT_NET_H__