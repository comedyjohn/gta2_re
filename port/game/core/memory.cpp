#include "core/memory.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

// 0x4FE4D0
void* Memory::Malloc(size_t Size)
{
    void* result = ::malloc(Size);
    if (!result)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 44, Size);
    }

    return result;
}

// 0x4FE510
void* Memory::AlignedMalloc(s32 requestedSize, void** pUnAlignedPtr)
{
    // Allocate with extra space for alignment
    void* pAlloc = ::malloc(requestedSize + 0x10000);

    // Return the raw unaligned allocation (since we can't free the aligned one)
    *pUnAlignedPtr = pAlloc;

    // Fail if no alloc
    if (!pAlloc)
    {
        FatalError_4A38C0(Gta2Error::OutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 76, requestedSize + 0x10000);
    }

    // Zeroed so that reads past the chunk the caller fills are defined (valgrind found them in the tile reader)
    memset(pAlloc, 0, requestedSize + 0x10000);

    // Return aligned pointer
    return reinterpret_cast<void*>((reinterpret_cast<sptr>(*pUnAlignedPtr) + 0xFFFF) & ~0xFFFF);
}