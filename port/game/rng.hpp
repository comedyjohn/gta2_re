#pragma once

#include "Function.hpp"

class Rng
{
  public:
    Rng()
    {
        rng_ = 0;
        rnd_ = 0;
    }

    s32 GetCurRng()
    {
        return rng_;
    }

    s32 GetRnd()
    {
        return rnd_;
    }

    void AdvanceCycle();
    void ShowCycle();

    void Srand();
    s16 GetInt(const s16& max_rnd);
    u8 GetUint8(const u8& max_rnd);
    int Rand();

    s32 rng_;
    s32 rnd_;
};

EXTERN_GLOBAL(Rng*, gpRng_67AB34);

EXTERN_GLOBAL(Rng, gRng_6F6784);
