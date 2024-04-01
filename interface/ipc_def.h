#ifndef __IPC_DEF_H__
#define __IPC_DEF_H__

/**
 * @brief
 *
 */

#include "ipc_utility.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <string>

#ifdef _MSC_VER
#define __throw__(x) throw std::exception(x);
#define __FNC__      __FUNCSIG__
#define IPC_HANDLE(...)           \
    catch (std::exception & ex) { \
        std::cerr << ex.what();   \
        __VA_ARGS__;              \
    }
#else
#define __throw__(x) throw std::string(x);
#define __FNC__      __PRETTY_FUNCTION__
#define IPC_HANDLE(...)           \
    catch (std::string & ex) {    \
        std::cerr << ex;          \
        __VA_ARGS__;              \
    }                             \
    catch (std::exception & ex) { \
        std::cerr << ex.what();   \
        __VA_ARGS__;              \
    }
#endif

#define IPC_THROW(x)                            \
    std::stringstream str;                      \
    str << "\033[1;31m" << __FNC__              \
        << "(" << __LINE__ << ")"               \
        << ": " << x << "\033[0m" << std::endl; \
    __throw__(str.str().c_str());

#define USE_IPC_LOG

#ifdef USE_IPC_LOG
#define IPC_INFO(...)  printf("[IPC_INFO] - " __VA_ARGS__);
#define IPC_ERROR(...) printf("[IPC_ERROR] - " __VA_ARGS__);
#else
#define IPC_INFO(...)
#define IPC_ERROR(...)
#endif

#define IPC_OK  0
#define IPC_ERR -1

namespace ipc {

}

#endif // __IPC_DEF_H__