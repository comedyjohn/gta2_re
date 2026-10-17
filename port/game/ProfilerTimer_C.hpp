#pragma once

#include "core/function.hpp"

class ProfilerTimer_C
{
  public:
    void AccumulateElapsed_5BEBF0();

    ProfilerTimer_C();
    ~ProfilerTimer_C() = default; // 0x5BEC20

    s32 field_0_time_percent;
    s32 field_4_start_time;
    s32 field_8_accum_time;
};