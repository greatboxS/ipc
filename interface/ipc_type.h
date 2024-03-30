#ifndef __IPC_TYPE_H__
#define __IPC_TYPE_H__

#include <functional>
#include <string>
#include <memory>

namespace gbs {
namespace ipc {
class IpcMessage;
typedef std::function<void(const std::string &topic, const std::string &publisher, std::shared_ptr<IpcMessage> message)> IpcTopicCallback;
} // namespace ipc

} // namespace gbs

#endif // __IPC_TYPE_H__