#ifndef __BACKTRACE_H__
#define __BACKTRACE_H__

namespace except {
#if defined(_UNIX) || defined(LINUX)
// this will make it run before all other static constructor functions
void backtraceInit(); // __attribute__((constructor(101)));

#elif defined(_WIN32) && defined(_MSC_VER)
// this will make it run before all other static constructor functions
#pragma warning(push)
#pragma warning(disable : 4074)
#pragma init_seg(compiler)

void backtraceInit();
#pragma warning(pop)

#elif defined(_WIN32) && (defined(__MINGW32__) || defined(__MINGW64__))
// this will make it run before all other static constructor functions
void backtraceInit(); // __attribute__((constructor(101)));
#endif
} // namespace except

#endif