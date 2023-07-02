#ifndef __IPC_PORT_DBUS_H__
#define __IPC_PORT_DBUS_H__

#include "dbus/dbus_client_adaptor.h"
#include "ipc_port.h"
#include "ipc_type.h"
#include <memory>
#include <sdbus-c++/sdbus-c++.h>
#include <shared_mutex>
#include <unordered_map>

namespace gbs
{
    namespace ipc
    {
        namespace dbus
        {
            class ServerProxyInterface;
        } // namespace dbus

        class IpcDbusPort : public IpcPort,
                            private dbus::ClientManagerAdaptor,
                            private dbus::ClientAdaptor
        {
        private:
            int m_id;
            std::shared_mutex m_mtx;
            const IpcProfile *m_profile;
            dbus::ServerProxyInterface &m_serverProxyInterface;
            std::queue<IpcMessage *> m_messageQueue;
            std::unordered_map<std::string, std::vector<IpcTopicCallback>> m_subscribers;

        public:
            explicit IpcDbusPort(const IpcProfile *profile,
                                 dbus::ServerProxyInterface &proxy,
                                 sdbus::IConnection &connection, std::string path, std::string name);
            virtual ~IpcDbusPort() {}

            virtual int id() const { return m_id; }
            virtual const IpcProfile *profile() const { return m_profile; }
            virtual const std::shared_ptr<IpcMessage> read();
            virtual int write(const IpcMessage &msg);
            virtual int subscribe(std::string &topic, IpcTopicCallback callback);
            virtual int unSubscribe(std::string &topics);
            virtual int publishMessage(std::string &topic, const IpcMessage &message);

        private:
            virtual int32_t sendMessage(const std::string &sender, const std::vector<uint8_t> &message) final;
            virtual int32_t publishMessage(const std::string &sender, const std::string &topic, const std::vector<uint8_t> &message) final;
        };
    } // namespace ipc

} // namespace gbs

#endif // __IPC_PORT_DBUS_H__