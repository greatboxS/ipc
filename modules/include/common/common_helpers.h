#ifndef COMMON_HELPERS_H
#define COMMON_HELPERS_H

#include <exception>
#include <iostream>
#include <string>
#include <sstream>

namespace ipc {

enum class ipcerror {
    Error = -1,
    Success = 0,
};

namespace exp {

inline void throw_str_exp(const std::string &exp, const char *function = __PRETTY_FUNCTION__, uint64_t line = __LINE__) {
    std::stringstream str = {};
    str << "\033[1;31m" << function << "(" << line << ")" << ": " << exp << "\033[0m" << std::endl;
    throw std::string(str.str().c_str());
}

} // namespace exp

namespace core {
} // namespace core

} // namespace ipc

#endif // COMMON_HELPERS_H