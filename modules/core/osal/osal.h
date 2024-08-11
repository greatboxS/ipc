/**
 * @file OSAL.h
 * @author greatboxsS (greatboxS@gmail.com)
 * @brief
 *         Operating system abtractrion layer
 * @version 0.1
 * @date 2022-08-11
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef OSAL_H
#define OSAL_H

#include <string.h>
#include <errno.h>
#include <stdio.h>

#if defined(WIN32) || defined(_WIN32)
namespace ipc::core {
__DLL_DECLSPEC__ extern const char *GetLastErrorStr();

} // namespace ipc::core

#define __ERROR__     GetLastError()
#define __ERROR_STR__ GetLastErrorStr()
#else
#define __ERROR__     errno
#define __ERROR_STR__ strerror(errno)
#endif

#if defined(OSAL_DEBUG)
#define OSAL_INFO printf
#define OSAL_ERR(...) fprintf(stderr, __VA_ARGS__)
#else
#define OSAL_INFO
#define OSAL_ERR(...)

#endif

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

#include <cstddef>
#include <cstdint>

/* ------------------------------ SOCKET DEFINITION ------------------------------ */
#if (defined(WIN32) || defined(_WIN32))
#include <Winsock2.h>
#include <ws2ipdef.h>
#include <Windows.h>

#define SOCKADDR_V4 SOCKADDR_IN
#define SOCKADDR_V6 SOCKADDR_IN6
#define SOCKADDR_H  SOCKADDR_V4

#elif (defined(LINUX) || defined(__linux__))
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKADDR_V4    sockaddr_in
#define SOCKADDR_V6    sockaddr_in6
#define SOCKADDR_H     sockaddr_un
#define SOCKET         int
#define INVALID_SOCKET -1

#else

#endif
typedef union {
    SOCKADDR_H un;
    SOCKADDR_V4 v4;
    SOCKADDR_V6 v6;
} SocketGenericIpAddr_t;

typedef struct __INetSocketAddress_t {
    SocketGenericIpAddr_t Ip;
    uint32_t u32Size;
    int32_t s32AddrFamily;
} INetSocketAddress_t;

typedef struct __Socket_t {
    SOCKET skHandle;
    int32_t s32Error;
    int32_t s32SocketType;
    int32_t s32BlockMode;
    const __Socket_t *pstHostSocket;
    INetSocketAddress_t stAddrInet;
} Socket_t;

typedef struct __SocketOption_t {
    int32_t s32Level;
    int32_t s32Option;
    void *pBuffer;
    size_t uSize;
} SocketOption_t;

#define SOCKET_T   Socket_t
#define SOCKET_OPT SocketOption_t
#define SOCKADDR_T SocketGenericIpAddr_t

#define SOCKET_BLOCKING_MODE    0
#define SOCKET_NONBLOCKING_MODE 1
#define SOCKET_ADDR_V4          AF_INET
#define SOCKET_ADDR_V6          AF_INET6

typedef enum __eSocketType {
    eSOCKET_HOST = 0,
    eSOCKET_TCP,
    eSOCKET_UDP
} eSocketType;

typedef enum __eSocketMode {
    eSOCKET_CLIENT = 0,
    eSOCKET_SERVER,
} eSocketMode;
/* ------------------------------ SOCKET DEFINITION ------------------------------ */

/* ------------------------------ SHARED MEMORY DEFINITION ---------------------- */
#define SHM_NAME_SIZE 256

#if (defined(WIN32) || defined(_WIN32))
#include <Windows.h>
#define shm_t  HANDLE
#define TIME_T DWORD
#elif (defined(LINUX) || defined(__linux__))
#include "sys/mman.h"
#define shm_t  int
#define TIME_T long int

#else

#endif

typedef struct __SHM_t {
    shm_t handle;
    char name[SHM_NAME_SIZE];
    void *virt;
    void *phys;
    size_t size;
} SHM_t;

#define SHM_T SHM_t
/* ------------------------------ SHARED MEMORY DEFINITION ---------------------- */

/* ------------------------------ MUTEX DEFINITION ------------------------------ */
#define MTX_NAME_SIZE 256
#if (defined(WIN32) || defined(_WIN32))
#include <synchapi.h>
#define mutex_t HANDLE

#elif (defined(LINUX) || defined(__linux__))
#include <pthread.h>
#define mutex_t pthread_mutex_t *

#else

#endif // WINDOWS

/* Named mutex ís allocated by using shared memory*/

typedef struct {
    mutex_t lock;
    char name[MTX_NAME_SIZE];
    SHM_T mem;
} MUTEX_t;

#define MUTEX_T MUTEX_t
/* ------------------------------ MUTEX DEFINITION ------------------------------ */

/* ------------------------------ SEMAPHORE DEFINITION -------------------------- */
#define SEM_NAME_SIZE          256
#define SEM_DEFAULT_INIT_VALUE 10
#if (defined(WIN32) || defined(_WIN32))
#define sem_t HANDLE

#elif (defined(LINUX) || defined(__linux__))
#include <semaphore.h>

#else

#endif

typedef struct __SEM_t {
#if (defined(WIN32) || defined(_WIN32))
    sem_t handle;
#else
    sem_t *handle;
#endif
    char name[SEM_NAME_SIZE];
} SEM_t;

#define SEM_T SEM_t
/* ------------------------------ SEMAPHORE DEFINITION -------------------------- */

/* ------------------------------ MESG QUEUE DEFINITION ------------------------- */
#define MSG_QUEUE_NAME_LEN 64

#if (defined(WIN32) || defined(_WIN32))
#include "queue/Queue.h"

#elif (defined(LINUX) || defined(__linux__))
#include "mqueue.h"

#ifdef SHARED_MEMORY_MESSAGE_QUEUE
#include "queue/Queue.h"
#endif

#else

#endif

typedef struct __MSGQ_t {
#if !defined(WIN32) && !defined(_WIN32)
    int handle;
#else
    HANDLE handle;
#endif
    size_t msgsize;
    size_t msgcount;
    size_t currcount;
    char mqname[MSG_QUEUE_NAME_LEN];
#if defined(WIN32) || defined(_WIN32) || defined(SHARED_MEMORY_MESSAGE_QUEUE)
    SHM_t shm;
    SEM_T sem;
    MUTEX_T mtx;
    Queue *que;
#endif
} MSGQ_t;

#define MSGQ_T MSGQ_t
/* ------------------------------ MESG QUEUE DEFINITION ------------------------- */

/* ------------------------------ THREAD DEFINITION ----------------------------- */
#define TASK_ARG       void *
#define TASK_NAME_SIZE 256
/* Default stack size is 1 MB */
#define THREAD_DEFAULT_STACK_SIZE (1024 * 10 * 1024 * 1)
#define VOID_T                    void *

#if (defined(WIN32) || defined(_WIN32))
#include <Processthreadsapi.h>
#define TASK_T   HANDLE
#define TASK_RET DWORD

#elif (defined(LINUX) || defined(__linux__))
#include <pthread.h>
#define TASK_T   pthread_t *
#define TASK_RET void *

#else

#endif

typedef enum __eTHREAD_Result {
    THREAD_NO_ERROR = 0,
    THREAD_FAILED,
    THREAD_INVALID_ARG,
    TRHEAD_INVALID_NAME,
} eTHREAD_Result;

typedef TASK_RET (*THREAD_FunctionCall)(TASK_ARG);

typedef struct __THREAD_t {
    THREAD_FunctionCall fnc;
    char name[TASK_NAME_SIZE];
    TASK_T task;
    int core;
    int priority;
} THREAD_t;

#define THREAD_T THREAD_t

/* ------------------------------ THREAD DEFINITION ----------------------------- */

/* ------------------------------ FILE DEFINITION ------------------------------- */

#if (defined(WIN32) || defined(_WIN32))
#define FILE_T HANDLE

#elif (defined(LINUX) || defined(__linux__))

#include <fcntl.h>
#include <unistd.h>
#define FILE_T int

#else

#endif
/* ------------------------------ FILE DEFINITION -------------------------------- */

/* ------------------------------ TIMER DEFINITION ------------------------------- */
#define TIMER_NAME_SIZE 256

typedef void (*TIMER_Callback)(void *);
typedef struct __TIMER_t {
    int id;
    int interval;
    void *param;
    char name[TIMER_NAME_SIZE];
    TIMER_Callback fnc;
} TIMER_t;

#define TIMER_T TIMER_t
/* ------------------------------ TIMER DEFINITION ------------------------------ */

/* ------------------------------ PROCESS DEFINITION ------------------------------ */
#define PROCESS_NAME_SIZE 256
#define PROCESS_ARGV_SIZE 512

#if (defined(WIN32) || defined(_WIN32))
#define process_t HANDLE

#elif (defined(LINUX) || defined(__linux__))
#define process_t pid_t

#else

#endif

typedef struct __Process_t {
    process_t handle;
    char name[PROCESS_NAME_SIZE];
#ifdef _WIN32
    HANDLE hJobObject;
    PROCESS_INFORMATION stProcessInfo;
#endif
    int32_t UseExternalConsole;
    int32_t s32HighPrioriry;
    char argv[PROCESS_ARGV_SIZE];
} Process_t;

#define PROCESS_T Process_t
/* ------------------------------ PROCESS DEFINITION ------------------------------ */

#ifndef RET_OK
#define RET_OK 0
#endif

#ifndef RET_ERR
#define RET_ERR -1
#endif

#endif // OSAL_H