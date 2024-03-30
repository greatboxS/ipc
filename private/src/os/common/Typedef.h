#ifndef __TYPEDEF_H__
#define __TYPEDEF_H__

#if (defined(_WIN32) || defined(WIN32))

#define std_str(s) std::string(s)

#if defined(_DLL) && !defined(STATIC_LIBRARY)
/* Windows DLL build */
#if defined(DLL_LIB_BUILD) && !defined(APP_BUILD)
#define __DLL_DECLSPEC__ __declspec(dllexport)
#else
/* Windows application build */
#define __DLL_DECLSPEC__ __declspec(dllimport)
#endif

#define __DLL_EXPORT__ __declspec(dllexport)
#define __DLL_IMPORT__ __declspec(dllimport)

#else
#define __DLL_DECLSPEC__
#endif

#else
#define std_str(s) s
#define __DLL_DECLSPEC__
#define __DLL_EXPORT__ __attribute__((__visibility__("default")))
#define __DLL_IMPORT__
#endif

#include <cstddef>
#include <cstdint>

#endif // __TYPEDEF_H__