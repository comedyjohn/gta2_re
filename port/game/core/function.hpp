#pragma once

#include "Globals.hpp"
#include "core/types.hpp"
#include <cstddef>
#include <cstdio>

#define DEFINE_GLOBAL(type, name, addr) type name
#define DEFINE_GLOBAL_INIT(type, name, value, addr) type name = value

#define DEFINE_GLOBAL_ARRAY(type, name, size, addr) type name[size]
#define DEFINE_GLOBAL_ARRAY_INIT(type, name, size, addr, ...) type name[size] = {__VA_ARGS__}

#define EXTERN_GLOBAL(type, name) extern type name
#define EXTERN_GLOBAL_ARRAY(type, name, size) extern type name[size]

// For a structure laid over data read from a file, whose size must not change at 64 bits either
#define GTA2_ASSERT_FILE_SIZEOF(structureName, expectedSize) \
    static_assert(sizeof(structureName) == expectedSize, "sizeof(" #structureName ") must be " #expectedSize);

// Only accepts arrays (a pointer does not compile), unlike sizeof(x) / sizeof(*x)
template<class T, std::size_t N>
constexpr std::size_t Gta2CountOf(const T (&)[N])
{
    return N;
}

#define GTA2_COUNTOF(x) (Gta2CountOf(x))
#define GTA2_COUNTOF_S(x) (static_cast<s32>(Gta2CountOf(x)))

#define GTA2_DELETE_AND_NULL(x) \
    delete x;                   \
    x = nullptr;
