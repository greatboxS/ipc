#ifndef __IPC_CONFIG_H__
#define __IPC_CONFIG_H__

#include <string>
#include <unordered_map>
namespace ipc {
namespace ipc {
class IpcConfiguration {
    std::unordered_map<std::string, std::string> m_prameters;

public:
    explicit IpcConfiguration(const std::string &file) {
    }
    ~IpcConfiguration() {
    }
    const std::string getItem(const std::string &key) const {
        std::string ret;
        if (m_prameters.find(key) == m_prameters.end()) return ret;
        return m_prameters.at(key);
    }
};
} // namespace ipc

} // namespace ipc

#endif // __IPC_CONFIG_H__