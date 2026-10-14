// Force-included before every translation unit of the native (non-MSVC) build.
// Supplies what MSVC provides implicitly, so the game sources compile unchanged.
#pragma once

#if defined(_MSC_VER)
    #error "port/include is for the native build only; the VC6 build uses the real Windows headers"
#endif

#if __SIZEOF_POINTER__ == 8
    // 64-bit (GTA2_BITS=64): pointers the game kept in 32-bit integers and file structures holding
    // pointers are handled (sptr, PTR32, the loaders' 64-bit branches), but it hasn't been played
    // yet. See port/TODO.md.
    #define GTA2_PORT_64BIT 1
#elif __SIZEOF_POINTER__ != 4
    #error "Unsupported pointer size"
#endif

#if __SIZEOF_WCHAR_T__ != 2
    // GTA2's text is UTF-16 and several structures hold wchar_t arrays.
    #error "Build with -fshort-wchar so wchar_t is 2 bytes, as on Windows"
#endif

#define GTA2_PORT 1

// Third-party headers the game includes (GTA2Hax's ddraw_vc6.h, Miles' mss.h) pick their Win32 branch
// from these. They are defined for game code only: the SDL platform layer (port/platform/) is compiled
// without this prelude, since SDL itself would take them to mean "building for Windows".
#define _WIN32 1
#define WIN32 1
#define _X86_ 1

// Every C runtime header is included up front, before port_wchar.h and port_crt.h rename functions
// with macros: libstdc++'s <cstdlib>/<cwchar> re-export names with using-declarations that a later
// include would otherwise see renamed.
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#ifdef __cplusplus
    #include <cstddef>
    #include <cstdlib>
    #include <cstring>
    #include <cstdio>
    #include <cstdarg>
    #include <cwchar>
    #include <cmath>
    #include <ctime>
    #include <cctype>
    #include <new>
    #include <string>
    #include <vector>
    #include <set>
    #include <map>
    #include <algorithm>
    #include <fstream>
    #include <sstream>
    #include <memory>
#endif

// gbh_graphics.hpp declares placeholder std::vector/std::set unless MSVC's <vector>/<set> include
// guards are set (it keeps VC6's codegen matching). The native build uses the real ones.
#define _SET_
#define _VECTOR_

// MSVC keywords and extensions. __stdcall is defined exactly as Function.hpp's non-MSVC branch does.
#define __stdcall
#define _stdcall
#define _cdecl
#define __cdecl
#define __fastcall
#define __thiscall
#define __forceinline inline
#define __declspec(x)
#define __int8 char
#define __int16 short
#define __int32 int
#define __int64 long long

#include "port_wchar.h"
#include "port_crt.h"

#ifdef __cplusplus
    #include <windows.h>
#endif
