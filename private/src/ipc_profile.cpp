#ifndef __PROFILE_H__
#define __PROFILE_H__

#include "ipc_profile.h"
#include <map>
#include <string>

namespace gbs {
namespace ipc {
static int32_t sId = 0;
static std::map<std::string, int32_t> sServices;

int32_t IpcProfile::getServiceId(const std::string &name) {
    auto id = sServices.find(name);
    if (id != sServices.end()) return id->second;

    sId++;
    sServices[name] = sId;
    return sId;
}

const std::string IpcProfile::getServiceName(int32_t id) {
    for (auto &s : sServices) {
        if (s.second == id) return s.first;
    }
    return "";
}
} // namespace ipc
} // namespace gbs

#endif // __PROFILE_H__