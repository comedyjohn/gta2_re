#pragma once

#include "Function.hpp"

class Rng
{
  public:
    Rng()
    {
        field_0_rng = 0;
        field_4_rnd = 0;
    }

    s32 GetCurRng()
    {
        return field_0_rng;
    }

    s32 GetRnd()
    {
        return field_4_rnd;
    }

    void AdvanceCycle();
    void ShowCycle();

    void Srand();
    s16 GetInt(const s16& max_rnd);
    u8 GetUint8(const u8& max_rnd);
    int Rand();

    s32 field_0_rng;
    s32 field_4_rnd;
};

EXTERN_GLOBAL(Rng*, gpRng_67AB34);

EXTERN_GLOBAL(Rng, gRng_6F6784);
