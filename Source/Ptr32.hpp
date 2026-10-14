#pragma once

#include "types.hpp"

// PTR32(T): a pointer the game keeps in 4 bytes of data it loaded from a file. The mission script
// is one block read from the .scr file, and its commands keep the objects they create (a car, a
// ped, a thread, ...) in a 4-byte slot of the command itself, with the next field right after it.
//
// In the original, and in a 32-bit native build, it is the pointer itself. In the 64-bit native build
// (port/) a pointer doesn't fit, so it is a 32-bit handle that port/platform/ptr32.cpp maps to the
// pointer: the slot keeps its size and the script data its layout. Handle 0 is the null pointer, so
// a slot the file leaves zeroed reads as null.
#if defined(GTA2_PORT_64BIT)

u32 Ptr32_ToHandle(const void* p);
void* Ptr32_FromHandle(u32 handle);

template <typename T>
struct Ptr32
{
    // No constructors, so it can sit in the unions of the script structures
    u32 field_0_handle;

    operator T*() const
    {
        return static_cast<T*>(Ptr32_FromHandle(field_0_handle));
    }

    T* operator->() const
    {
        return *this;
    }

    // An explicit cast to another pointer type, as the code does with the raw pointer
    template <typename U>
    explicit operator U*() const
    {
        return (U*)Ptr32_FromHandle(field_0_handle);
    }

    Ptr32& operator=(T* p)
    {
        field_0_handle = Ptr32_ToHandle(p);
        return *this;
    }
};

    #define PTR32(T) Ptr32<T>
#else
    #define PTR32(T) T*
#endif
