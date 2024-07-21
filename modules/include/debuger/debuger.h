#ifndef DEBUGER_H
#define DEBUGER_H

#include <string.h>
#include <errno.h>
#include <stdio.h>

#if (defined(_WIN32) || defined(WIN32))

#define std_str(s) std::string(s)

#if defined(_DLL) && !defined(STATIC_LIBRARY)
/* Windows DLL build */
#if defined(DLL_LIB_BUILD) && !defined(APP_BUILD)
#define __dll_declspec__ __declspec(dllexport)
#else
/* Windows application build */
#define __dll_declspec__ __declspec(dllimport)
#endif

#define __DLL_EXPORT__ __declspec(dllexport)
#define __DLL_IMPORT__ __declspec(dllimport)

#else
#define __dll_declspec__
#endif

#else
#define std_str(s) s
#define __dll_declspec__
#define __DLL_EXPORT__ __attribute__((__visibility__("default")))
#define __DLL_IMPORT__
#endif

#if defined(WIN32) || defined(_WIN32)
#define __ERROR__ GetLastError()
#else
#define __ERROR__     errno
#define __ERROR_STR__ strerror(errno)
#endif

typedef enum __eDebugLevel {
    DBG_LVL_NONE = 0,
    DBG_LVL_INFO = 1,
    DBG_LVL_WARN = 2,
    DBG_LVL_TRACE = 4,
    DBG_LVL_ERROR = 8,
    DBG_LVL_ALL = 0xFFFFFFFF,
} eDebugLevel;

__dll_declspec__ extern int __DEBUG_LEVEL__;

/**
 * @fn DBG_SetLevel
 * @brief Set debug log level\
 *
 * Use DBG_LVL_INFO | DBG_LVL_WARN ... to enable specific log.
 * Use DBG_LVL_ALL to enable all log
 * Use DBG_LVL_NONE to disable all log
 */
__dll_declspec__ extern void DBG_SetLevel(int);

#include <assert.h>
#if defined(DEBUG) || defined(_DEBUG)
#ifdef DBG_FLUSH_ALWAYS
#define DBG_PRINT(...)   \
    printf(__VA_ARGS__); \
    fflush(stdout);
#else
#define DBG_PRINT(...) printf(__VA_ARGS__);
#endif

#if defined(WIN32) || defined(_WIN32)
#include <typeinfo>
#define __CLASS_NAME__ typeid(decltype(*this)).name()
#else
extern const char *DBG_GetClassName(const char *);
#define __CLASS_NAME__ DBG_GetClassName(typeid(decltype(*this)).name())
#endif

#else
#define DBG_PRINT(...)
#endif
#define __FORMAT_RESET "\033[0m"
#define __COLOR_RESET  __FORMAT_RESET
/* Foreground color */
#define __F_WHITE "\033[1;37m"
#define __F_CYAN  "\033[1;36m"
#define __F_PINK  "\033[1;35m"
#define __F_BLUE  "\033[1;34m"
#define __F_YELOW "\033[1;33m"
#define __F_GREEN "\033[1;32m"
#define __F_RED   "\033[1;31m"
#define __F_GREY  "\033[1;30m"
#define __F_BLACK "\033[30m"
#define __F_NONE  __COLOR_RESET
/* Background color */
#define __B_GREY  "\033[47m"
#define __B_WHITE "\033[47m"
#define __B_CYAN  "\033[46m"
#define __B_PINK  "\033[45m"
#define __B_BLUE  "\033[44m"
#define __B_YELOW "\033[43m"
#define __B_GREEN "\033[42m"
#define __B_RED   "\033[41m"
#define __B_BLACK "\033[40m"
#define __B_NONE  __COLOR_RESET

#define __BOLD           "\033[1m"
#define __ITALIC         "\033[3m"
#define __STRIKE_THROUGH "\033[9m"
#define __UNDER_LINE     "\033[4m"

#if defined(WIN32) || defined(_WIN32)
__dll_declspec__ extern void
ActivateVirtualTerminal();
#ifdef USE_VIRTUAL_TERMINAL
#define __INFO  "[" __F_GREY "INFO " __F_NONE "]"
#define __WARN  "[" __F_YELOW "WARN " __F_NONE "]"
#define __TRACE "[" __F_BLUE "TRACE" __F_NONE "]"
#define __ERROR "[" __F_RED "ERROR" __F_NONE "]"
#else
#define __INFO  "[INFO ]"
#define __WARN  "[WARN ]"
#define __TRACE "[TRACE]"
#define __ERROR "[ERROR]"

#define __COLOR_RESET
/* Foreground color */
#define __F_WHITE
#define __F_CYAN
#define __F_PINK
#define __F_BLUE
#define __F_YELOW
#define __F_GREEN
#define __F_RED
#define __F_GREY
#define __F_BLACK
#define __F_NONE __COLOR_RESET
/* Background color */
#define __B_GREY
#define __B_WHITE
#define __B_CYAN
#define __B_PINK
#define __B_BLUE
#define __B_YELOW
#define __B_GREEN
#define __B_RED
#define __B_BLACK
#define __B_NONE __COLOR_RESET

#define __BOLD
#define __ITALIC
#define __STRIKE_THROUGH
#define __UNDER_LINE

#endif

#else
#define __INFO  "[" __F_GREY "INFO " __F_NONE "]"
#define __WARN  "[" __F_YELOW "WARN " __F_NONE "]"
#define __TRACE "[" __F_BLUE "TRACE" __F_NONE "]"
#define __ERROR "[" __F_RED "ERROR" __F_NONE "]"
#endif

/* Color format */
#define __FORMAT(text, format) format text __COLOR_RESET

#define __LOG_DEV(level, tag, layer, ...)                              \
    if (__DEBUG_LEVEL__ & level) {                                     \
        DBG_PRINT(layer "%s-[%s-%d] : ", tag, __FUNCTION__, __LINE__); \
        DBG_PRINT(__VA_ARGS__);                                        \
    }

#define LOG_INFO(layer, ...)  __LOG_DEV(DBG_LVL_INFO, __INFO, layer, __VA_ARGS__)
#define LOG_WARN(layer, ...)  __LOG_DEV(DBG_LVL_WARN, __WARN, layer, __VA_ARGS__)
#define LOG_TRACE(layer, ...) __LOG_DEV(DBG_LVL_TRACE, __TRACE, layer, __VA_ARGS__)
#define LOG_ERROR(layer, ...) __LOG_DEV(DBG_LVL_ERROR, __ERROR, layer, __VA_ARGS__)

/**
 * @brief DEFAULT LOGGING
 *
 */
#define CLOG_INFO(...)                                                                                                                  \
    if (__DEBUG_LEVEL__ & DBG_LVL_INFO) {                                                                                               \
        DBG_PRINT("[CLASS]: %s-[" __FORMAT("%s", __F_CYAN __UNDER_LINE) "][%s-%d] : ", __INFO, __CLASS_NAME__, __FUNCTION__, __LINE__); \
        DBG_PRINT(__VA_ARGS__);                                                                                                         \
    }
#define CLOG_WARN(...)                                                                                                                  \
    if (__DEBUG_LEVEL__ & DBG_LVL_WARN) {                                                                                               \
        DBG_PRINT("[CLASS]: %s-[" __FORMAT("%s", __F_CYAN __UNDER_LINE) "][%s-%d] : ", __WARN, __CLASS_NAME__, __FUNCTION__, __LINE__); \
        DBG_PRINT(__VA_ARGS__);                                                                                                         \
    }
#define CLOG_TRACE(...)                                                                                                                  \
    if (__DEBUG_LEVEL__ & DBG_LVL_TRACE) {                                                                                               \
        DBG_PRINT("[CLASS]: %s-[" __FORMAT("%s", __F_CYAN __UNDER_LINE) "][%s-%d] : ", __TRACE, __CLASS_NAME__, __FUNCTION__, __LINE__); \
        DBG_PRINT(__VA_ARGS__);                                                                                                          \
    }
#define CLOG_ERROR(...)                                                                                                                  \
    if (__DEBUG_LEVEL__ & DBG_LVL_ERROR) {                                                                                               \
        DBG_PRINT("[CLASS]: %s-[" __FORMAT("%s", __F_CYAN __UNDER_LINE) "][%s-%d] : ", __ERROR, __CLASS_NAME__, __FUNCTION__, __LINE__); \
        DBG_PRINT(__VA_ARGS__);                                                                                                          \
    }

#define NONE_EMBED
#define EXIT_IF(state, ret, embed_func) \
    if (state) {                        \
        embed_func;                     \
        return ret;                     \
    }

namespace ipc::core {
/* If the BUILT_IN_BACKTRACE is defined, the backtrace will be init automatically,
 * instead of call manually by user
 */
#ifndef BUILT_IN_BACKTRACE
#if defined(_UNIX) || defined(__linux__)
void backtrace_init();

#elif defined(_WIN32) && defined(_MSVC)
#pragma warning(push)
#pragma warning(disable : 4074)
#pragma init_seg(compiler)

void backtrace_init();
#pragma warning(pop)

#elif defined(_WIN32) && defined(_MINGW)
void backtrace_init();
#endif
#endif
} // namespace ipc::core

#endif // DEBUGER_H