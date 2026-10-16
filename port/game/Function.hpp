#pragma once

#include "Globals.hpp"
#include "types.hpp"
#include <cstddef>
#include <cstdio>

// Variadic macros were added in C99, but this is not supported by VC6.
// We need to use variadic args for the DEFINE_GLOBAL_ARRAY_INIT macro.
// If we use a regular parameter instead of the ellipsis and ignore the
// C4002 "too many parameters for macro" warning, this will not properly
// initialize the array. An array of structs will fail to compile at all.
// This leads us to this ugly hack to include commas in macro expansion
// but not confuse the compiler with extra parameters.
#define COMMA ,


// Report unfinished code the first time the game reaches it
#define NOT_IMPLEMENTED                                        \
    do                                                         \
    {                                                          \
        static bool logged___ = false;                         \
        if (!logged___)                                        \
        {                                                      \
            logged___ = true;                                  \
            printf("NOT IMPLEMENTED %s\n", __PRETTY_FUNCTION__); \
        }                                                      \
    } while (0)

#define DEFINE_GLOBAL(type, name, addr) type name
#define DEFINE_GLOBAL_INIT(type, name, value, addr) type name = value

#define DEFINE_GLOBAL_ARRAY(type, name, size, addr) type name[size]
#define DEFINE_GLOBAL_ARRAY_INIT(type, name, size, addr, va_args) type name[size] = {va_args}

#define EXTERN_GLOBAL(type, name) extern type name
#define EXTERN_GLOBAL_ARRAY(type, name, size) extern type name[size]

// The size a structure has in the original. It counts 4-byte pointers, so the 64-bit build skips it.
#if __SIZEOF_POINTER__ == 8
    #define GTA2_ASSERT_SIZEOF_ALWAYS(structureName, expectedSize)
#else
    #define GTA2_ASSERT_SIZEOF_ALWAYS(structureName, expectedSize) \
        static_assert(sizeof(structureName) == expectedSize, "sizeof(" #structureName ") must be " #expectedSize);
#endif

// For a structure laid over data read from a file, whose size must not change at 64 bits either
#define GTA2_ASSERT_FILE_SIZEOF(structureName, expectedSize) \
    static_assert(sizeof(structureName) == expectedSize, "sizeof(" #structureName ") must be " #expectedSize);

// Only accepts arrays (a pointer does not compile), unlike sizeof(x) / sizeof(*x)
template <class T, std::size_t N>
constexpr std::size_t Gta2CountOf(const T (&)[N])
{
    return N;
}

#define GTA2_COUNTOF(x) (Gta2CountOf(x))
#define GTA2_COUNTOF_S(x) (static_cast<s32>(Gta2CountOf(x)))

#define GTA2_DELETE_AND_NULL(x) \
    delete x;                   \
    x = nullptr;
