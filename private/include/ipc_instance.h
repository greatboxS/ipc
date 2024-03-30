#ifndef __IPC_INSTANCE_H__
#define __IPC_INSTANCE_H__

#include "ipc_profile.h"
#include "ipc_proxy.h"
#include <memory>

namespace gbs {
namespace ipc {
class IpcIMessage;
class IpcAbstractInstance : public IpcProfile {
private:
protected:
    std::unique_ptr<IpcProxy> m_proxy;
    std::string m_name;
    AppId m_appId;
    ProfileId m_profileId;
    GroupId m_groupId;
    IpcType m_type;

    void eventLoop(const void *argv);

public:
    explicit IpcAbstractInstance(IpcType type) :
        m_type(type) {}
    virtual ~IpcAbstractInstance() {}
    /* Profile interface*/
    virtual ProfileId id() const { return m_profileId; }
    virtual const std::string &name() const { return m_name; }
    virtual GroupId groupId() const { return m_groupId; }
    virtual AppId appId() const { return m_appId; }

    /* Proxy interface*/
    int connect() { return m_proxy->connect(); }
    int disconnect() { return m_proxy->disconnect(); }
    bool connected() { return m_proxy->connected(); }
    IpcPort *port() { return m_proxy->port(); }

    virtual int onConnected() = 0;
    virtual int onDisconnected() = 0;
    virtual int onDataIn(const IpcIMessage *msg) = 0;
};
} // namespace ipc

} // namespace gbs

#endif // __IPC_IN  STANCE_H__