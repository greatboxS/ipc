#ifndef __IPC_PROXY_NET_H__
#define __IPC_PROXY_NET_H__

#include "ipc_proxy.h"

namespace gbs {
namespace ipc {
class IpcNetPort;

class IpcNetProxy : public IpcProxy {
private:
public:
    explicit IpcNetProxy();
    virtual ~IpcNetProxy();

    virtual int connect();
    virtual int disconnect();
    virtual bool connected();
    virtual IpcPort *port();
};
} // namespace ipc
} // namespace gbs

#endif // __IPC_PROXY_NET_H__