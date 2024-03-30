#ifndef __IPC_PROXY_SHM_H__
#define __IPC_PROXY_SHM_H__

#include "ipc_proxy.h"

namespace gbs {
namespace ipc {
class IpcShmPort;

class IpcShmProxy : public IpcProxy {
private:
public:
    explicit IpcShmProxy();
    virtual ~IpcShmProxy();

    virtual int connect();
    virtual int disconnect();
    virtual bool connected();
    virtual IpcPort *port();
};
} // namespace ipc
} // namespace gbs

#endif // __IPC_PROXY_SHM_H__