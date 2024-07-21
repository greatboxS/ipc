#include <inttypes.h>
#include <typeinfo>
#include <initializer_list>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "debuger/debuger.h"

#if defined(__linux__)
#include <unistd.h>
#if defined(_MACOS)
#define AM_PTHREAD_T_FMT "%p"
#else
#define AM_PTHREAD_T_FMT "%lx"
#endif
#elif defined(_WIN32)
#include <io.h>
#if !defined(STDERR_FILENO)
#define STDERR_FILENO _fileno(stderr)
#endif
#define write(a, b, c) _write(a, b, static_cast<unsigned int>(c))
#if defined(_MINGW)
#include <cxxabi.h>
#endif
#endif

#define BACKTRACE_BUFFER_SIZE 1000
#define BACKTRACE_LINE_SIZE   2048
#define _UNIX

static bool print_backtrace = false;
static bool dump_core = false;
static int wait_for_gdb_attach = 0;

static char *demangle_buff = NULL;
static size_t demangle_buff_size = 0;
static std::string *backtrace_line_out = NULL;

#if defined(_WIN32) || (defined(_UNIX) && !defined(_ANDROID))

/* If the BUILT_IN_BACKTRACE is defined, the backtrace will be init automatically,
 * instead of call manually by user
 */
#ifdef BUILT_IN_BACKTRACE
#if defined(_UNIX) || defined(__linux__)
void backtrace_init() __attribute__((constructor(101)));
static void backtrace_unix_init();

#elif defined(_WIN32) && defined(_MSVC)
#pragma warning(push)
#pragma warning(disable : 4074)
#pragma init_seg(compiler)

static void backtrace_windown_init();
void backtrace_init();
static struct InitBacktrace {
    InitBacktrace() { except::backtrace_init(); }
} sbackTrace;
#pragma warning(pop)

#elif defined(_WIN32) && defined(_MINGW)
static void backtrace_windown_init();
void backtrace_init() __attribute__((constructor(101)));
#endif

#else

#if defined(_UNIX) || defined(__linux__)
static void backtrace_unix_init();

#elif defined(_WIN32) && defined(_MSVC)
// this will make it run before all other static constructor functions
#pragma warning(push)
#pragma warning(disable : 4074)
#pragma init_seg(compiler)

static void backtrace_windown_init();

#pragma warning(pop)

#elif defined(_WIN32) && defined(_MINGW)
static void backtrace_windown_init();
#endif

#endif

namespace ipc::core {
void backtrace_init() {
    print_backtrace = true;
    dump_core = true;
    wait_for_gdb_attach = 0;

    demangle_buff_size = 768;
    demangle_buff = static_cast<char *>(malloc(demangle_buff_size));
    backtrace_line_out = new std::string();
    backtrace_line_out->reserve(BACKTRACE_BUFFER_SIZE);

#if defined(_UNIX)
    backtrace_unix_init();
#elif defined(_WIN32)
    backtrace_windown_init();
#endif
}
} // namespace ipc::core

static void backtrace_printf(FILE *stream, const char *msg, int msgLen = -1) {
    if (stream == stderr) {
        auto dummy = write(STDERR_FILENO, msg, msgLen >= 0 ? size_t(msgLen) : strlen(msg));
        dummy = write(STDERR_FILENO, "\n", 1);
    }
}

static void backtrace_print_line(FILE *stream, int level, const char *symbol, uintptr_t offset = 0,
                                 const char *file = nullptr, int line = -1,
                                 int errorCode = 0, const char *errorString = nullptr) {

    bool isError = (errorString);
    bool lineValid = (line > 0 && line <= 9999999);

    backtrace_line_out->clear();
    if (isError) {
        backtrace_line_out->append("ERROR: ");
        backtrace_line_out->append(errorString);
        backtrace_line_out->append(" (");
        backtrace_line_out->append(std::to_string(errorCode));
        backtrace_line_out->append(")");
    } else {
        backtrace_line_out->append("\t- ");
        backtrace_line_out->append(symbol ? symbol : "?");
    }
    if (offset) {
        backtrace_line_out->append(" [");
        backtrace_line_out->append(std::to_string(static_cast<long long>(offset)));
        backtrace_line_out->append("]");
    }
    if (file) {
        if (lineValid)
            backtrace_line_out->append(std::to_string(lineValid ? line : 0));

        backtrace_line_out->append(" in ");
        static const char *filePrefix =
#if defined(_WIN32)
            "file:///";
#else
            "file://";
#endif

        bool isFileUrl = (strncmp(file, filePrefix, sizeof(filePrefix) - 1) == 0);
        if (!isFileUrl)
            backtrace_line_out->append(filePrefix);
        backtrace_line_out->append((isFileUrl) ? file + sizeof(filePrefix) - 1 : file);

        if (lineValid) {
            backtrace_line_out->append(":");
            backtrace_line_out->append(std::to_string(lineValid ? line : 0));
        }
    }

    backtrace_printf(stream, backtrace_line_out->c_str());
}
#endif // defined(_WIN32) || (defined(_UNIX) && !defined(_ANDROID))

#if defined(_UNIX) && !defined(_ANDROID)

#include <cxxabi.h>
#include <execinfo.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <stdio.h>
#include <exception>
#include <iostream>
#include <fstream>

#if defined(__linux__)
#include <dlfcn.h>
#elif defined(_MACOS)
#include <mach-o/dyld.h>
#endif

const std::vector<int> sigs = {SIGFPE, SIGSEGV, SIGILL, SIGBUS, SIGPIPE, SIGABRT, /*SIGINT,*/ SIGQUIT, SIGSYS};

static void crash_handler(const char *why, int stackFramesToIgnore);

const char *signal_name(int sig) {
#if defined(_UNIX)
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 32
    return sigdescr_np(sig);
#else
    return strsignal(sig); // not async-signal-safe
#endif
#else
    return "<unknown>";
#endif
}

static void signal_handler(int sig) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "uncaught signal %d (%s)", sig, signal_name(sig));
    // 8 means to remove 8 stack frames: this way the backtrace starts at the point where
    // the signal reception interrupted the normal program flow
    crash_handler(buffer, 8);
}

static void backtrace_unix_init() {
#if defined(__linux__)
    dlopen("libgcc_s.so.1", RTLD_GLOBAL | RTLD_LAZY);
#endif

    for (auto sig : sigs) {
        signal(sig, signal_handler);
    }
    std::set_terminate([]() {
        char buffer[1024];

        auto type = abi::__cxa_current_exception_type();
        if (!type) {
            // 3 means to remove 3 stack frames: this way the backtrace starts at std::terminate
            crash_handler("terminate was called although no exception was thrown", 3);
        }

        const char *typeName = type->name();
        if (typeName) {
            int status;
            demangle_buff = abi::__cxa_demangle(typeName, demangle_buff, &demangle_buff_size, &status);
            if (status == 0 && *demangle_buff) {
                typeName = demangle_buff;
            }
        }
        try {
            throw;
        } catch (const std::exception &exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc.what());
        } catch (const std::exception *exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc->what());
        } catch (const char *exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type 'const char *' (%s)", exc);
        } catch (...) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s", typeName);
        }

        // 4 means to remove 4 stack frames: this way the backtrace starts at std::terminate
        crash_handler(buffer, 4);
    });

    // create a new process group, so that we are able to kill all children with ::kill(0, ...)
    setpgid(0, 0);
}

static void log_crash_info(FILE *stream, const char *why, int stackFramesToIgnore) {
    char title[256];
    char who[256];

    pid_t pid = getpid();
    snprintf(title, sizeof(title), "/proc/%d/cmdline", pid);

    std::ifstream cmdLineFile(title);
    std::string cmdLine;
    std::getline(cmdLineFile, cmdLine);

    if (!title) {
        ssize_t whoLen = readlink("/proc/self/exe", who, sizeof(who) - 1);
        who[std::max(ssize_t(0), whoLen)] = '\0';
        memcpy(title, who, sizeof(title));
    }

#if defined(__linux__)
    long tid = syscall(SYS_gettid);
    bool is_main_thread = (tid == pid);
#else
    long tid = -1;
    bool is_main_thread = pthread_main_np();
#endif
    pthread_t pthread_id = pthread_self();
    char thread_name[128];

    if (is_main_thread)
        strcpy(thread_name, "main");
    else if (pthread_getname_np(pthread_id, thread_name, sizeof(thread_name)))
        strcpy(thread_name, "unknown");

    fprintf(stream, "\n*** process %s (%d) crashed ***", cmdLine.c_str(), pid);
    fprintf(stream, "\n > why: %s", why);
    fprintf(stream, "\n > where: %s thread, TID: %ld, pthread ID: " AM_PTHREAD_T_FMT, thread_name, tid, pthread_id);

    void *addr_array[1024];
    int addr_count = backtrace(addr_array, sizeof(addr_array) / sizeof(*addr_array));

    if (!addr_count) {
        backtrace_printf(stream, " > no C++ backtrace available");
    } else {
        char **symbols = backtrace_symbols(addr_array, addr_count);

        if (!symbols) {
            backtrace_printf(stream, " > no symbol names available");
        } else {
            backtrace_printf(stream, "\n > C++ backtrace:");
            for (int i = 1; i < addr_count; ++i) {
                char *function = nullptr;
                char *offset = nullptr;
                char *end = nullptr;

                for (char *ptr = symbols[i]; ptr && *ptr; ++ptr) {
                    if (!function && *ptr == '(')
                        function = ptr + 1;
                    else if (function && !offset && *ptr == '+')
                        offset = ptr;
                    else if (function && !end && *ptr == ')')
                        end = ptr;
                }

                const char *name = nullptr;

                if (function && offset && end && (function != offset)) {
                    *offset = 0;
                    *end = 0;

                    int status;
                    demangle_buff = abi::__cxa_demangle(function, demangle_buff, &demangle_buff_size, &status);
                    name = (status == 0 && *demangle_buff) ? demangle_buff : function;
                } else {
                    name = symbols[i];
                    if (function && (function == offset))
                        *(function - 1) = 0;
                }
                backtrace_print_line(stream, i, name, offset ? strtoull(offset + 1, nullptr, 16) : 0);
            }
        }
    }
}

static void crash_handler(const char *why, int stackFramesToIgnore) {
    // We also need to reset all the "crash" signals plus SIGINT for three reasons:
    //  1) avoid recursions
    //  2) SIGABRT to re-enable standard abort() handling
    //  3) SIGINT, so that you can Ctrl+C the app if the crash handler ends up freezing

    for (int sig : sigs) {
        signal(sig, ((__sighandler_t)0));
    }

    log_crash_info(stderr, why, stackFramesToIgnore);

    if (wait_for_gdb_attach > 0) {
        fprintf(stderr, "\n > the process will be suspended for %d seconds and you can attach a debugger"
                        " to it via\n\n   gdb -p %d\n",
                wait_for_gdb_attach, getpid());
        static jmp_buf jmpenv;
        signal(SIGALRM, [](int) {
            longjmp(jmpenv, 1);
        });
        if (!setjmp(jmpenv)) {
            alarm(static_cast<unsigned int>(wait_for_gdb_attach));

            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGALRM);
            sigsuspend(&mask);
        } else {
            backtrace_printf(stderr, "\n > no gdb attached\n");
        }
    }

    // make sure to terminate our sub-process as well, but not ourselves
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    if (dump_core) {
        backtrace_printf(stderr, "\n > the process will be aborted (core dumped)\n");
        abort();
    }

    _exit(-1);
}

#elif defined(_WIN32)

#include <windows.h>
#include <dbghelp.h>

static DWORD mainThreadId = GetCurrentThreadId();

static void log_crash_info(FILE *stream, const char *why, int stackFramesToIgnore, CONTEXT *context) {
    WCHAR title[256];
    DWORD titleLen = sizeof(title) / sizeof(*title);
    if (QueryFullProcessImageName(GetCurrentProcess(), /*PROCESS_NAME_NATIVE*/ 0, title, &titleLen)) {
        title[qMax(DWORD(0), titleLen)] = '\0';
    }

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    WCHAR threadName[256];
    // Qt uses the the legacy RaiseException mechanism to attach the thread name instead of the
    // modern setThreadDescription() call, but this name is only visible to debuggers.
    if (tid == mainThreadId) {
        wcscpy(threadName, L"main");
    } else {
        wcscpy(threadName, L"unknown");

        // GetThreadDescription is only available from Windows 10, 1607 onwards, regardless of the
        // SDK used to compile this code
        typedef HRESULT(WINAPI * GetThreadDescriptionType)(HANDLE hThread, PWSTR * description);
        if (auto GetThreadDescriptionFunc = reinterpret_cast<GetThreadDescriptionType>(
                reinterpret_cast<QFunctionPointer>(
                    GetProcAddress(::GetModuleHandle(L"kernel32.dll"),
                                   "GetThreadDescription")))) {
            WCHAR *desc = nullptr;
            if (SUCCEEDED(GetThreadDescriptionFunc(GetCurrentThread(), &desc)))
                wcscpy_s(threadName, sizeof(threadName) / sizeof(*threadName), desc);
        }
    }

    fprintf(stream, "\n*** process %S (%lu) crashed ***", title, pid);
    fprintf(stream, "\n > why: %s", why);
    fprintf(stream, "\n > where: %S thread, TID: %lu", threadName, tid);

    if (context) {
        backtrace_printf(stream, "\n > C++ backtrace: cannot be generated"
#if defined(_MSVC)
                                 ", StackWalker is not enabled"
#elif defined(_MINGW)
                                 ", MinGW is not supported"
#endif
        );
    }
}

#if defined(_MSVC)
// From Dr. Dobbs: https://www.drdobbs.com/visual-c-exception-handling-instrumentat/184416600
// These types are known to the compiler without declaration.

// This had been adapted for 64bit and 32bit platforms
// all pointers are not real pointers on 64bit Windows, but 32bit offsets relative to HINSTANCE!

struct _PMD {
    qint32 mdisp;
    qint32 pdisp;
    qint32 vdisp;
};

// information of a type that can be caught by a catch block
struct _s__CatchableType {
    quint32 properties; // type properties bit flags: 1 - type is a pointer
    quint32 /* type info * */ pType;
    _PMD thisDisplacement;                 // displacement of this from the beginning of the object
    qint32 sizeOrOffset;                   // size of the type
    quint32 /* void (*)() */ copyFunction; // != 0 if the type has a copy constructor
};

// a counted array of the types that can be caught by a catch block
struct _s__CatchableTypeArray {
    qint32 nCatchableTypes;
#pragma warning(push)
#pragma warning(disable : 4200)
    quint32 /* const _s__CatchableType * */ arrayOfCatchableTypes[];
#pragma warning(pop)
};

// this structure holds the information about the exception object being thrown
struct _s__ThrowInfo {
    quint32 attributes;              // 1: ptr to const obj / 2: ptr to volatile obj
    quint32 /* void * */ pmfnUnwind; // exception object destructor
    quint32 /* void * */ pForwardCompat;
    // array of sub-types of the object or other types to which it could be implicitly cast.
    // Used to find the matching catch block
    const quint32 /* const _s__CatchableTypeArray * */ pCatchableTypeArray;
};
#endif // defined(_MSVC)

#define EXCEPTION_CPP_EXCEPTION   0xE06D7363 // internal MSVC value
#define EXCEPTION_MINGW_EXCEPTION 0xE014e9aa // custom AM value

static LONG WINAPI windowsExceptionFilter(EXCEPTION_POINTERS *ep) {
    static char buffer[1024];
    bool suppressBacktrace = false;
    int stackFramesToIgnore = 0;

    switch (ep->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION: strcpy(buffer, "access violation"); break;
    case EXCEPTION_DATATYPE_MISALIGNMENT: strcpy(buffer, "datatype misalignment"); break;
    case EXCEPTION_BREAKPOINT: strcpy(buffer, "breakpoint"); break;
    case EXCEPTION_SINGLE_STEP: strcpy(buffer, "single step"); break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: strcpy(buffer, "array bounds exceeded"); break;
    case EXCEPTION_FLT_DENORMAL_OPERAND: strcpy(buffer, "denormal float operand"); break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO: strcpy(buffer, "float divide-by-zero"); break;
    case EXCEPTION_FLT_INEXACT_RESULT: strcpy(buffer, "inexact float result"); break;
    case EXCEPTION_FLT_INVALID_OPERATION: strcpy(buffer, "invalid float operation"); break;
    case EXCEPTION_FLT_OVERFLOW: strcpy(buffer, "float overflow"); break;
    case EXCEPTION_FLT_STACK_CHECK: strcpy(buffer, "float state check"); break;
    case EXCEPTION_FLT_UNDERFLOW: strcpy(buffer, "float underflow"); break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO: strcpy(buffer, "integer divide-by-zero"); break;
    case EXCEPTION_INT_OVERFLOW: strcpy(buffer, "integer overflow"); break;
    case EXCEPTION_PRIV_INSTRUCTION: strcpy(buffer, "private instruction"); break;
    case EXCEPTION_IN_PAGE_ERROR: strcpy(buffer, "in-page error"); break;
    case EXCEPTION_ILLEGAL_INSTRUCTION: strcpy(buffer, "illegal instruction"); break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION: strcpy(buffer, "noncontinuable exception"); break;
    case EXCEPTION_STACK_OVERFLOW:
        strcpy(buffer, "stack overflow");
        suppressBacktrace = true;
        break;
    case EXCEPTION_INVALID_DISPOSITION: strcpy(buffer, "invalid disposition"); break;
    case EXCEPTION_GUARD_PAGE: strcpy(buffer, "guard page"); break;
    case EXCEPTION_INVALID_HANDLE: strcpy(buffer, "invalid handle"); break;
#if defined(_MINGW)
    case EXCEPTION_MINGW_EXCEPTION: {
        if ((ep->ExceptionRecord->NumberParameters == 1) && (ep->ExceptionRecord->ExceptionInformation[0])) {
            strcpy_s(buffer, sizeof(buffer),
                     reinterpret_cast<const char *>(ep->ExceptionRecord->ExceptionInformation[0]));
        }
        break;
    }
#elif defined(_MSVC)
    case EXCEPTION_CPP_EXCEPTION: {
        // MSVC bug: std::current_exception is null, even though the standard says it shouldn't
        // be in this situation (it even is null in a set_terminate() handler).
        // So we have to use the compiler internal structure and functions to (a) get the type-name
        // of the exception and to (b) re-throw it in order to print the what() of a std::exception
        // derived instance.

        // (a) get the type_info

        std::type_info *type = nullptr;
        bool is64bit = (sizeof(void *) == 8);
        if (ep->ExceptionRecord->NumberParameters == (is64bit ? 4 : 3)) {
            auto hInstance = (is64bit ? reinterpret_cast<char *>(ep->ExceptionRecord->ExceptionInformation[3])
                                      : nullptr);

            // since all "pointers" are 32bit values even on 64bit Windows, we have to add the
            // hInstance segment pointer to each of the 32bit pointers to get a real 64bit address
            if (!is64bit || hInstance) {
                const auto *ti = reinterpret_cast<_s__ThrowInfo *>(ep->ExceptionRecord->ExceptionInformation[2]);
                if (ti) {
                    auto cta = reinterpret_cast<_s__CatchableTypeArray *>(hInstance + ti->pCatchableTypeArray);
                    if (cta && (cta->nCatchableTypes > 0) && (cta->nCatchableTypes < 100)) {
                        auto ct = reinterpret_cast<_s__CatchableType *>(hInstance + cta->arrayOfCatchableTypes[0]);
                        if (ct)
                            type = reinterpret_cast<std::type_info *>(hInstance + ct->pType);
                    }
                }
            }
        }
        const char *typeName = type ? type->name() : "<unknown type>";

        // (b) re-throw and catch the exception
        try {
            RaiseException(EXCEPTION_CPP_EXCEPTION,
                           EXCEPTION_NONCONTINUABLE,
                           ep->ExceptionRecord->NumberParameters,
                           ep->ExceptionRecord->ExceptionInformation);
        } catch (const std::exception &exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc.what());
        } catch (const std::exception *exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc->what());
        } catch (const char *exc) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type 'const char *' (%s)", exc);
        } catch (...) {
            snprintf(buffer, sizeof(buffer), "uncaught exception of type %s", typeName);
        }

        stackFramesToIgnore = 2; // CxxThrowException + RaiseException
        break;
    }
#endif // defined(_MSVC)
    default:
        snprintf(buffer, sizeof(buffer), "unknown Windows exception, code: %lx",
                 ep->ExceptionRecord->ExceptionCode);
        break;
    }

    log_crash_info(stderr, buffer, stackFramesToIgnore,
                 suppressBacktrace ? nullptr : ep->ContextRecord);

    if (Logging::deferredMessages()) {
        backtrace_printf(stderr, "\n > Accumulated logging output\n");
        Logging::completeSetup();
    }

    TerminateProcess(GetCurrentProcess(), ep->ExceptionRecord->ExceptionCode);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void backtrace_windown_init() {
    // create a "process group", so that child process are automatically killed if this process dies
    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits;
    memset(&limits, 0, sizeof(limits));
    limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    AssignProcessToJobObject(hJob, GetCurrentProcess());

    // handle Windows' Structured Exceptions
    SetUnhandledExceptionFilter(windowsExceptionFilter);

#if defined(_MINGW)
    // MinGW does handle the exceptions like gcc does on Unix, instead of using a structured
    // Windows exception, so we have to adapt:

    std::set_terminate([]() {
        char buffer[1024];

        auto type = abi::__cxa_current_exception_type();
        if (!type) {
            strcpy(buffer, "terminate was called although no exception was thrown");
        } else {
            const char *typeName = type->name();
            if (typeName) {
                int status;
                demangle_buff = abi::__cxa_demangle(typeName, demangle_buff, &demangle_buff_size, &status);
                if (status == 0 && *demangle_buff) {
                    typeName = demangle_buff;
                }
            }
            try {
                throw;
            } catch (const std::exception &exc) {
                snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc.what());
            } catch (const std::exception *exc) {
                snprintf(buffer, sizeof(buffer), "uncaught exception of type %s (%s)", typeName, exc->what());
            } catch (const char *exc) {
                snprintf(buffer, sizeof(buffer), "uncaught exception of type 'const char *' (%s)", exc);
            } catch (...) {
                snprintf(buffer, sizeof(buffer), "uncaught exception of type %s", typeName);
            }
        }
        ULONG_PTR args[1] = {reinterpret_cast<ULONG_PTR>(buffer)};
        RaiseException(EXCEPTION_MINGW_EXCEPTION, EXCEPTION_NONCONTINUABLE, 1, args);
    });
#endif // defined(_MINGW)
}

#endif // defined(_WIN32)