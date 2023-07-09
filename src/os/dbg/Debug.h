#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "Exception.h"
#include "common/Typedef.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32)
__DLL_DECLSPEC__ extern const char *GetLastErrorStr();
#define __ERROR__     GetLastError()
#define __ERROR_STR__ GetLastErrorStr()
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

__DLL_DECLSPEC__ extern int __DEBUG_LEVEL__;

/**
 * @fn DBG_SetLevel
 * @brief Set debug log level\
 *
 * Use DBG_LVL_INFO | DBG_LVL_WARN ... to enable specific log.
 * Use DBG_LVL_ALL to enable all log
 * Use DBG_LVL_NONE to disable all log
 */
__DLL_DECLSPEC__ extern void DBG_SetLevel(int);

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
__DLL_DECLSPEC__ extern void ActivateVirtualTerminal();
#else
extern const char *DBG_GetClassName(const char *);
#define __CLASS_NAME__ DBG_GetClassName(typeid(decltype(*this)).name())
#endif

#else
#define DBG_PRINT(...)
#endif

#if defined(USE_VIRTUAL_TERMINAL) || defined(LINUX)
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

#else
#define __FORMAT_RESET
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

#if defined(WIN32) || defined(_WIN32)

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

#define __LOG_DEV_INFO(layer, ...)  __LOG_DEV(DBG_LVL_INFO, __INFO, layer, __VA_ARGS__)
#define __LOG_DEV_WARN(layer, ...)  __LOG_DEV(DBG_LVL_WARN, __WARN, layer, __VA_ARGS__)
#define __LOG_DEV_TRACE(layer, ...) __LOG_DEV(DBG_LVL_TRACE, __TRACE, layer, __VA_ARGS__)
#define __LOG_DEV_ERROR(layer, ...) __LOG_DEV(DBG_LVL_ERROR, __ERROR, layer, __VA_ARGS__)

/**
 * @brief OSAC LOGGING
 *
 */
#define LOG_INFO(...)  __LOG_DEV_INFO("[NOR]: ", __VA_ARGS__)
#define LOG_WARN(...)  __LOG_DEV_WARN("[NOR]: ", __VA_ARGS__)
#define LOG_TRACE(...) __LOG_DEV_TRACE("[NOR]: ", __VA_ARGS__)
#define LOG_ERROR(...) __LOG_DEV_ERROR("[NOR]: ", __VA_ARGS__)

/**
 * @brief OSAC LOGGING
 *
 */
#define LOG_OSAC_INFO(...)  __LOG_DEV_INFO("[OSAC]: ", __VA_ARGS__)
#define LOG_OSAC_WARN(...)  __LOG_DEV_WARN("[OSAC]: ", __VA_ARGS__)
#define LOG_OSAC_TRACE(...) __LOG_DEV_TRACE("[OSAC]: ", __VA_ARGS__)
#define LOG_OSAC_ERROR(...) __LOG_DEV_ERROR("[OSAC]: ", __VA_ARGS__)

/**
 * @brief OSAL LOGGING
 *
 */
#define LOG_OSAL_INFO(...)  __LOG_DEV_INFO("[OSAL]: ", __VA_ARGS__)
#define LOG_OSAL_WARN(...)  __LOG_DEV_WARN("[OSAL]: ", __VA_ARGS__)
#define LOG_OSAL_TRACE(...) __LOG_DEV_TRACE("[OSAL]: ", __VA_ARGS__)
#define LOG_OSAL_ERROR(...) __LOG_DEV_ERROR("[OSAL]: ", __VA_ARGS__)

/**
 * @brief SERVICE LOGGING
 *
 */
#define LOG_SERVICE_INFO(...)  __LOG_DEV_INFO("[SERVICE]: ", __VA_ARGS__)
#define LOG_SERVICE_WARN(...)  __LOG_DEV_WARN("[SERVICE]: ", __VA_ARGS__)
#define LOG_SERVICE_TRACE(...) __LOG_DEV_TRACE("[SERVICE]: ", __VA_ARGS__)
#define LOG_SERVICE_ERROR(...) __LOG_DEV_ERROR("[SERVICE]: ", __VA_ARGS__)

/**
 * @brief MAIN LOGGING
 *
 */
#define LOG_MAIN_INFO(...)  __LOG_DEV_INFO("[MAIN]: ", __VA_ARGS__)
#define LOG_MAIN_WARN(...)  __LOG_DEV_WARN("[MAIN]: ", __VA_ARGS__)
#define LOG_MAIN_TRACE(...) __LOG_DEV_TRACE("[MAIN]: ", __VA_ARGS__)
#define LOG_MAIN_ERROR(...) __LOG_DEV_ERROR("[MAIN]: ", __VA_ARGS__)

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

#endif // __DEBUG_H__