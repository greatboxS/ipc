#include "Debug.h"
#include <stdio.h>
#include <string.h>

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
static int IsActivatedVirtualTerminal = 0;
#endif

#ifdef DBG_FLUSH_ALWAYS
static int StdoutBuffDisabled = 0;
#endif

int __DEBUG_LEVEL__ = (int)DBG_LVL_INFO;

void DBG_SetLevel(int level) {

#ifdef DBG_FLUSH_ALWAYS
    if (!StdoutBuffDisabled) {
        StdoutBuffDisabled = 1;
        setbuf(stdout, NULL);
    }
#endif
    __DEBUG_LEVEL__ = level;

#if defined(WIN32) || defined(_WIN32)
#ifdef USE_VIRTUAL_TERMINAL
    ActivateVirtualTerminal();
#endif
#endif
}

const char *DBG_GetClassName(const char *name) {
    size_t len = strlen(name);
    size_t index = 0;

    for (index = 0; index < len; index++)
        if (!(name[index] >= '0' && name[index] <= '9')) break;

    return &name[index];
}

#if defined(WIN32) || defined(_WIN32)
void ActivateVirtualTerminal() {
    if (!IsActivatedVirtualTerminal) {
        HANDLE handleOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD consoleMode;
        GetConsoleMode(handleOut, &consoleMode);
        consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        consoleMode |= DISABLE_NEWLINE_AUTO_RETURN;
        SetConsoleMode(handleOut, consoleMode);
        IsActivatedVirtualTerminal = 1;
    }
}

const char *GetLastErrorStr() {
    LPSTR buffer = nullptr;
    DWORD errorCode = GetLastError();
    static std::string errorString = "";
    size_t size = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&buffer,
        0,
        nullptr);
    if (size == 0 || buffer == nullptr) {
        return errorString.c_str();
    }

    errorString = std::string(buffer, size);
    // cut off "/r/n" character
    errorString.resize(errorString.size() - 2);
    LocalFree(buffer);
    return errorString.c_str();
}
#endif