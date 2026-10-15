#pragma once

#include "Function.hpp"

class Memory
{
  public:
    static void* malloc_4FE4D0(size_t Size);

    static void* Aligned_malloc_4FE510(s32 requestedSize, void** pAlignedSize);
};