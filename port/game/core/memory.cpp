#include "core/memory.hpp"
#include "core/enums.hpp"
#include "core/function.hpp"
#include "error.hpp"
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

// 0x4FE4D0
void* Memory::Malloc(size_t size)
{
    void* result = ::malloc(size);
    if (!result)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 44, size);
    }

    return result;
}

// 0x4FE510
void* Memory::AlignedMalloc(s32 requested_size, void** un_aligned_ptr)
{
    // Allocate with extra space for alignment
    void* alloc = ::malloc(requested_size + 0x10000);

    // Return the raw unaligned allocation (since we can't free the aligned one)
    *un_aligned_ptr = alloc;

    // Fail if no alloc
    if (!alloc)
    {
        FatalError_4A38C0(Gta2Error::kOutOfMemory, "C:\\Splitting\\Gta2\\Source\\Memory.cpp", 76, requested_size + 0x10000);
    }

    // Zeroed so that reads past the chunk the caller fills are defined (valgrind found them in the tile reader)
    memset(alloc, 0, requested_size + 0x10000);

    // Return aligned pointer
    return reinterpret_cast<void*>((reinterpret_cast<sptr>(*un_aligned_ptr) + 0xFFFF) & ~0xFFFF);
}