#include <inttypes.h>
#include <typeinfo>
#include <initializer_list>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include "Backtrace.h"

#define __FORMAT_RESET "\033[0m"
#define __COLOR_RESET  __FORMAT_RESET
#define __WHITE        "\033[1;37m"
#define __CYAN         "\033[1;36m"
#define __PINK         "\033[1;35m"
#define __BLUE         "\033[1;34m"
#define __YELOW        "\033[1;33m"
#define __GREEN        "\033[1;32m"
#define __RED          "\033[1;31m"
#define __GREY         "\033[1;30m"
#define __BLACK        "\033[30m"
#define __NONE         __COLOR_RESET

#if defined(LINUX)
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
#if defined(__MINGW32__) || defined(__MINGW64__)
#define _MINGW
#include <cxxabi.h>
#endif
#endif

#define BACKTRACE_BUFFER_SIZE 1000
#define BACKTRACE_LINE_SIZE   2048

static bool printBacktrace = false;
static bool printQmlStack = false;
static bool dumpCore = false;
static int waitForGdbAttach = 0;

static char *demangleBuffer = NULL;
static size_t demangleBufferSize = 0;
static std::string *backtraceLineOut = NULL;

#if defined(_WIN32) || (defined(LINUX) && !defined(_ANDROID))

#if defined(LINUX)
static void backtraceUnixInit();

static struct InitBacktrace {
    // InitBacktrace() { except::backtraceInit(); }
} dummy;

#elif defined(_WIN32) && defined(_MSC_VER)
// this will make it run before all other static constructor functions
#pragma warning(push)
#pragma warning(disable : 4074)
// #pragma init_seg(compiler)

static void backtraceWindowsInit();

#pragma warning(pop)

#elif defined(_WIN32) && defined(_MINGW)
static void backtraceWindowsInit();

#endif

namespace except {
void backtraceInit() {
    printBacktrace = true;
    printQmlStack = true;
    dumpCore = true;
    waitForGdbAttach = 0;

    demangleBufferSize = 768;
    demangleBuffer = static_cast<char *>(malloc(demangleBufferSize));
    backtraceLineOut = new std::string();
    backtraceLineOut->reserve(BACKTRACE_BUFFER_SIZE);

#if defined(LINUX)
    backtraceUnixInit();
#elif defined(_WIN32)
    backtraceWindowsInit();
#endif
}
} // namespace except

static void backtracePrintf(FILE *stream, const char *msg, int msgLen = -1) {
    if (stream == stderr) {
        auto dummy = write(STDERR_FILENO, msg, msgLen >= 0 ? size_t(msgLen) : strlen(msg));
        dummy = write(STDERR_FILENO, "\n", 1);
    }
}

static void backtracePrintLine(FILE *stream, int level, const char *symbol, uintptr_t offset = 0,
                               const char *file = nullptr, int line = -1,
                               int errorCode = 0, const char *errorString = nullptr) {

    bool isError = (errorString);
    bool lineValid = (line > 0 && line <= 9999999);

    backtraceLineOut->clear();
    if (isError) {
        backtraceLineOut->append("ERROR: ");
        backtraceLineOut->append(errorString);
        backtraceLineOut->append(" (");
        backtraceLineOut->append(std::to_string(errorCode));
        backtraceLineOut->append(")");
    } else {
        backtraceLineOut->append("\t- ");
        backtraceLineOut->append(symbol ? symbol : "?");
    }
    if (offset) {
        backtraceLineOut->append(" [");
        backtraceLineOut->append(std::to_string(static_cast<long long>(offset)));
        backtraceLineOut->append("]");
    }
    if (file) {
        if (lineValid)
            backtraceLineOut->append(std::to_string(lineValid ? line : 0));

        backtraceLineOut->append(" in ");
        static const char *filePrefix =
#if defined(_WIN32)
            "file:///";
#else
            "file://";
#endif

        bool isFileUrl = (strncmp(file, filePrefix, sizeof(filePrefix) - 1) == 0);
        if (!isFileUrl)
            backtraceLineOut->append(filePrefix);
        backtraceLineOut->append((isFileUrl) ? file + sizeof(filePrefix) - 1 : file);

        if (lineValid) {
            backtraceLineOut->append(":");
            backtraceLineOut->append(std::to_string(lineValid ? line : 0));
        }
    }

    backtracePrintf(stream, backtraceLineOut->c_str());
}
#endif // defined(_WIN32) || (defined(LINUX) && !defined(_ANDROID))

#if defined(LINUX) && !defined(_ANDROID)

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

#if defined(LINUX)
#include <dlfcn.h>
#elif defined(_MACOS)
#include <mach-o/dyld.h>
#endif

const std::vector<int> sigs = {SIGFPE, SIGSEGV, SIGILL, SIGBUS, SIGPIPE, SIGABRT, SIGINT, SIGQUIT, SIGSYS};

static void crashHandler(const char *why, int stackFramesToIgnore);

const char *signalName(int sig) {
#if defined(LINUX)
#if __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 32
    return sigdescr_np(sig);
#else
    return strsignal(sig); // not async-signal-safe
#endif
#else
    return "<unknown>";
#endif
}

static void signalHandler(int sig) {
    char buffer[256];
    snprintf(buffer, sizeof(buffer), "uncaught signal %d (%s)", sig, signalName(sig));
    // 8 means to remove 8 stack frames: this way the backtrace starts at the point where
    // the signal reception interrupted the normal program flow
    crashHandler(buffer, 8);
}

static void backtraceUnixInit() {
#if defined(LINUX)
    dlopen("libgcc_s.so.1", RTLD_GLOBAL | RTLD_LAZY);
#endif

    for (auto sig : sigs) {
        signal(sig, signalHandler);
    }
    std::set_terminate([]() {
        char buffer[1024];

        auto type = abi::__cxa_current_exception_type();
        if (!type) {
            // 3 means to remove 3 stack frames: this way the backtrace starts at std::terminate
            crashHandler("terminate was called although no exception was thrown", 3);
        }

        const char *typeName = type->name();
        if (typeName) {
            int status;
            demangleBuffer = abi::__cxa_demangle(typeName, demangleBuffer, &demangleBufferSize, &status);
            if (status == 0 && *demangleBuffer) {
                typeName = demangleBuffer;
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
        crashHandler(buffer, 4);
    });

    // create a new process group, so that we are able to kill all children with ::kill(0, ...)
    setpgid(0, 0);
}

static void logCrashInfo(FILE *stream, const char *why, int stackFramesToIgnore) {
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

#if defined(LINUX)
    long tid = syscall(SYS_gettid);
    bool isMainThread = (tid == pid);
#else
    long tid = -1;
    bool isMainThread = pthread_main_np();
#endif
    pthread_t pthreadId = pthread_self();
    char threadName[128];

    if (isMainThread)
        strcpy(threadName, "main");
    else if (pthread_getname_np(pthreadId, threadName, sizeof(threadName)))
        strcpy(threadName, "unknown");

    fprintf(stream, "\n*** process %s (%d) crashed ***", cmdLine.c_str(), pid);
    fprintf(stream, "\n > why: %s", why);
    fprintf(stream, "\n > where: %s thread, TID: %ld, pthread ID: " AM_PTHREAD_T_FMT,
            threadName, tid, pthreadId);

    if (printBacktrace) {
    }

    void *addrArray[1024];
    int addrCount = backtrace(addrArray, sizeof(addrArray) / sizeof(*addrArray));

    if (!addrCount) {
        backtracePrintf(stream, " > no C++ backtrace available");
        return;
    }

    char **symbols = backtrace_symbols(addrArray, addrCount);
    if (!symbols) {
        backtracePrintf(stream, " > no symbol names available");
        return;
    }

    backtracePrintf(stream, "\n > C++ backtrace:");
    for (int i = 1; i < addrCount; ++i) {
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
            demangleBuffer = abi::__cxa_demangle(function, demangleBuffer, &demangleBufferSize, &status);
            name = (status == 0 && *demangleBuffer) ? demangleBuffer : function;
        } else {
            name = symbols[i];
            if (function && (function == offset))
                *(function - 1) = 0;
        }
        backtracePrintLine(stream, i, name, offset ? strtoull(offset + 1, nullptr, 16) : 0);
    }
}

static void crashHandler(const char *why, int stackFramesToIgnore) {
    // We also need to reset all the "crash" signals plus SIGINT for three reasons:
    //  1) avoid recursions
    //  2) SIGABRT to re-enable standard abort() handling
    //  3) SIGINT, so that you can Ctrl+C the app if the crash handler ends up freezing

    for (int sig : sigs) {
        signal(sig, ((__sighandler_t)0));
    }

    logCrashInfo(stderr, why, stackFramesToIgnore);

    if (waitForGdbAttach > 0) {
        fprintf(stderr, "\n > the process will be suspended for %d seconds and you can attach a debugger"
                        " to it via\n\n   gdb -p %d\n",
                waitForGdbAttach, getpid());
        static jmp_buf jmpenv;
        signal(SIGALRM, [](int) {
            longjmp(jmpenv, 1);
        });
        if (!setjmp(jmpenv)) {
            alarm(static_cast<unsigned int>(waitForGdbAttach));

            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGALRM);
            sigsuspend(&mask);
        } else {
            backtracePrintf(stderr, "\n > no gdb attached\n");
        }
    }

    // make sure to terminate our sub-process as well, but not ourselves
    signal(SIGTERM, SIG_IGN);
    kill(0, SIGTERM);

    if (dumpCore) {
        backtracePrintf(stderr, "\n > the process will be aborted (core dumped)\n");
        abort();
    }

    _exit(-1);
}

#elif defined(_WIN32)

#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "Dbghelp.lib")

static DWORD mainThreadId = GetCurrentThreadId();

void backtracePrintf(FILE *stream, CONTEXT *context) {
    void *stack[100];
    WORD frames = CaptureStackBackTrace(0, 100, stack, NULL);

    if (!stream) return;

    DWORD64 pc = context ? context->Rip : 0;
    SYMBOL_INFO *symbol = (SYMBOL_INFO *)calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        fprintf(stream, "SymInitialize failed: %d\n", GetLastError());
        return;
    }

    fprintf(stream, "\n > C++ backtrace:\n");
    for (int i = 0; i < frames; i++) {
        DWORD64 address = (DWORD64)(stack[i]);
        DWORD displacement;
        IMAGEHLP_LINE64 lineInfo;

        SymFromAddr(GetCurrentProcess(), address, NULL, symbol);

        if (address != pc) continue;

        lineInfo.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        if (SymGetLineFromAddr64(GetCurrentProcess(), pc, &displacement, &lineInfo)) {
            fprintf(stream, __RED "# %s (0x%X) at: %s, line %d" __NONE "\n",
                    symbol->Name,
                    symbol->Address,
                    lineInfo.FileName,
                    lineInfo.LineNumber);
        } else {
            fprintf(stream, "#% %s (0x%X)\n", symbol->Name, symbol->Address);
            fprintf(stream, "Failed to get line information for address %p: \n", pc);
        }
        break;
    }

    SymCleanup(GetCurrentProcess());
    free(symbol);
}

static void logCrashInfo(FILE *stream, const char *why, int stackFramesToIgnore, CONTEXT *context) {
    char title[256];
    DWORD titleLen = sizeof(title) / sizeof(*title);
    if (QueryFullProcessImageNameA(GetCurrentProcess(), 0, title, &titleLen)) {
        title[max(DWORD(0), titleLen)] = '\0';
    }

    DWORD pid = GetCurrentProcessId();
    DWORD tid = GetCurrentThreadId();
    WCHAR threadName[256];
    // Qt uses the the legacy RaiseException mechanism to attach the thread name instead of the
    // modern setThreadDescription() call, but this name is only visible to debuggers.
    // GetThreadDescription is only available from Windows 10, 1607 onwards, regardless of the
    // SDK used to compile this code
    typedef HRESULT(WINAPI * GetThreadDescriptionType)(HANDLE hThread, PWSTR * description);
    if (auto GetThreadDescriptionFunc = reinterpret_cast<GetThreadDescriptionType>(GetProcAddress(::GetModuleHandleA("kernel32.dll"), "GetThreadDescription"))) {
        WCHAR *desc = nullptr;
        if (SUCCEEDED(GetThreadDescriptionFunc(GetCurrentThread(), &desc)))
            wcscpy_s(threadName, sizeof(threadName) / sizeof(*threadName), desc);
    }

    fprintf(stream, "\n*** process %S (%lu) crashed ***", title, pid);
    fprintf(stream, "\n > why: %s", why);
    fprintf(stream, "\n > where: %S thread, TID: %lu", threadName, tid);

    if (!stackFramesToIgnore)
        backtracePrintf(stream, context);
}

#if defined(_MSC_VER)
// From Dr. Dobbs: https://www.drdobbs.com/visual-c-exception-handling-instrumentat/184416600
// These types are known to the compiler without declaration.

// This had been adapted for 64bit and 32bit platforms
// all pointers are not real pointers on 64bit Windows, but 32bit offsets relative to HINSTANCE!

struct _PMD_t {
    int32_t mdisp;
    int32_t pdisp;
    int32_t vdisp;
};

// information of a type that can be caught by a catch block
struct _s__CatchableType_t {
    uint32_t properties; // type properties bit flags: 1 - type is a pointer
    uint32_t /* type info * */ pType;
    _PMD_t thisDisplacement;                // displacement of this from the beginning of the object
    int32_t sizeOrOffset;                   // size of the type
    uint32_t /* void (*)() */ copyFunction; // != 0 if the type has a copy constructor
};

// a counted array of the types that can be caught by a catch block
struct _s__CatchableTypeArray_t {
    int32_t nCatchableTypes;
#pragma warning(push)
#pragma warning(disable : 4200)
    uint32_t /* const _s__CatchableType_t * */ arrayOfCatchableTypes[];
#pragma warning(pop)
};

// this structure holds the information about the exception object being thrown
struct _s__ThrowInfo_t {
    uint32_t attributes;              // 1: ptr to const obj / 2: ptr to volatile obj
    uint32_t /* void * */ pmfnUnwind; // exception object destructor
    uint32_t /* void * */ pForwardCompat;
    // array of sub-types of the object or other types to which it could be implicitly cast.
    // Used to find the matching catch block
    const uint32_t /* const _s__CatchableTypeArray_t * */ pCatchableTypeArray;
};
#endif // defined(_MSC_VER)

#define EXCEPTION_CPP_EXCEPTION   0xE06D7363 // internal MSVC value
#define EXCEPTION_MINGW_EXCEPTION 0xE014e9aa // custom AM value

static LONG WINAPI windowsExceptionFilter(EXCEPTION_POINTERS *ep) {
    static char buffer[1024];
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
#elif defined(_MSC_VER)
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
                const auto *ti = reinterpret_cast<_s__ThrowInfo_t *>(ep->ExceptionRecord->ExceptionInformation[2]);
                if (ti) {
                    auto cta = reinterpret_cast<_s__CatchableTypeArray_t *>(hInstance + ti->pCatchableTypeArray);
                    if (cta && (cta->nCatchableTypes > 0) && (cta->nCatchableTypes < 100)) {
                        auto ct = reinterpret_cast<_s__CatchableType_t *>(hInstance + cta->arrayOfCatchableTypes[0]);
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
#endif // defined(_MSC_VER)
    default:
        snprintf(buffer, sizeof(buffer), "unknown Windows exception, code: %lx",
                 ep->ExceptionRecord->ExceptionCode);
        break;
    }

    logCrashInfo(stderr, buffer, stackFramesToIgnore, ep->ContextRecord);
    TerminateProcess(GetCurrentProcess(), ep->ExceptionRecord->ExceptionCode);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void backtraceWindowsInit() {
    // create a "process group", so that child process are automatically killed if this process dies
    // HANDLE hJob = CreateJobObject(nullptr, nullptr);
    // JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits;
    // memset(&limits, 0, sizeof(limits));
    // limits.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    // SetInformationJobObject(hJob, JobObjectExtendedLimitInformation, &limits, sizeof(limits));
    // AssignProcessToJobObject(hJob, GetCurrentProcess());

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
                demangleBuffer = abi::__cxa_demangle(typeName, demangleBuffer, &demangleBufferSize, &status);
                if (status == 0 && *demangleBuffer) {
                    typeName = demangleBuffer;
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
