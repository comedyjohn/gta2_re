#pragma once

#include "core/function.hpp"

class Memory
{
  public:
    static void* Malloc(size_t size);

    static void* AlignedMalloc(s32 requested_size, void** aligned_size);
};