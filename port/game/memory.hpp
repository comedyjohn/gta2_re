#pragma once

#include "Function.hpp"

class Memory
{
  public:
    static void* Malloc(size_t Size);

    static void* AlignedMalloc(s32 requestedSize, void** pAlignedSize);
};