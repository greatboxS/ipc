#ifndef __IPC_BROKER_NET_H__
#define __IPC_BROKER_NET_H__

#include "ipc_broker.h"
#include "os/osac/CSocket.h"
#include <memory>
#include <vector>
#include <queue>

namespace gbs {
namespace ipc {
class IpcProfile;

class IpcNetBroker : public IpcBrokerBase {
    const IpcProfile *m_profile;
    std::string m_host;
    uint16_t m_port;
    uint32_t m_connections;
    std::unique_ptr<osac::CSocket> m_socket;
    std::vector<std::shared_ptr<osac::CSocket>> m_clients;
    std::atomic<bool> m_recvRunning;
    std::shared_mutex m_localMtx;
    std::vector<std::unique_ptr<char>> m_recvBuffer;
    std::vector<std::unique_ptr<char>> m_msgBuffer;
    std::queue<char *> m_msgQueue;

    void recvEventLoop();

private:
    virtual int onBrokerStart();
    virtual int onBrokerStop();
    virtual int onBrokerRunning();
    virtual int onBrokerSend(const IpcProfile *profile, const std::string &message);
    virtual int onBrokerSubscribe(const IpcProfile *profile, const std::string &topic);
    virtual int onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic);
    virtual int onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message);

public:
    explicit IpcNetBroker(const IpcProfile *profile, const std::string &host, uint16_t port, uint32_t connections = 1000);
    virtual ~IpcNetBroker();
};

} // namespace ipc

} // namespace gbs

#endif // __IPC_BROKER_NET_H__