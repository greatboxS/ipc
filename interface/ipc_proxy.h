#ifndef __IPC_PROXY_H__
#define __IPC_PROXY_H__

namespace gbs {
namespace ipc {
class IpcProfile;
class IpcPort;
enum class IpcType;

class IpcProxy {
private:
public:
    virtual ~IpcProxy() = default;
    virtual int connect() = 0;
    virtual int disconnect() = 0;
    virtual bool connected() = 0;
    virtual IpcPort *port() = 0;
};
} // namespace ipc
} // namespace gbs

#endif // __IPC_PROXY_H__