#include "ipc_broker_net.h"
#include "ipc_profile.h"
#include <thread>
#include <mutex>
#include <future>

namespace gbs {
namespace ipc {
constexpr uint32_t RECV_BUFFER_SIZE = (1024 * 100);
constexpr int32_t RECV_BUFFER_COUNT = (32);
constexpr int32_t MSG_QUEUE_SIZE = (1000);

IpcNetBroker::IpcNetBroker(const IpcProfile *profile, const std::string &host, uint16_t port, uint32_t connections) :
    m_socket(std::make_unique<osac::CSocket>()),
    m_host(host), m_port(port), m_connections(connections),
    m_recvRunning(true) {
    for (int i = 0; i < RECV_BUFFER_COUNT; i++) {
        m_recvBuffer.push_back(std::unique_ptr<char>(new char[RECV_BUFFER_SIZE]));
    }

    for (int i = 0; i < MSG_QUEUE_SIZE; i++) {
        m_msgBuffer.push_back(std::unique_ptr<char>(new char[RECV_BUFFER_SIZE]));
    }
#ifndef _WIN32
    m_socket->Open((int32_t)osac::CSocket::Type::SocketHost, (int32_t)osac::CSocket::Mode::Server);
#else
#endif
    std::thread recvLoop(&IpcNetBroker::recvEventLoop, this);
}

IpcNetBroker::~IpcNetBroker() {
}

void IpcNetBroker::recvEventLoop() {
    int index = 0;
    char *buff = NULL;

    while (m_recvRunning) {
        {
            std::shared_lock<std::shared_mutex> lock(m_localMtx);
            for (auto &client : m_clients) {

                buff = m_recvBuffer[index].get();

                if (client->Receive(buff, RECV_BUFFER_SIZE) < 0) continue;

                std::async(std::launch::async, [buff]() {
                    /* Handle new incomming message */
                });
                index = ++index % RECV_BUFFER_COUNT;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
}

int IpcNetBroker::onBrokerStart() {
    m_socket->Bind(m_host.c_str(), m_port);
    m_socket->Listen(m_connections);
    return 0;
}
int IpcNetBroker::onBrokerStop() {
    return 0;
}

int IpcNetBroker::onBrokerRunning() {
    auto sk = m_socket->Accept();
    if (!sk) return 0;
    std::unique_lock<std::shared_mutex> lock(m_localMtx);
    m_clients.emplace_back(std::shared_ptr<osac::CSocket>(sk));
}

int IpcNetBroker::onBrokerSend(const IpcProfile *profile, const std::string &message) {
    return 0;
}

int IpcNetBroker::onBrokerSubscribe(const IpcProfile *profile, const std::string &topic) {
    return 0;
}

int IpcNetBroker::onBrokerUnSubscribe(const IpcProfile *profile, const std::string &topic) {
    return 0;
}

int IpcNetBroker::onBrokerPublicMessage(const IpcProfile *profile, const std::string &topic, const std::string &message) {
    return 0;
}

} // namespace ipc

} // namespace gbs
