// The game's random numbers: MSVC's rand() (port/platform/crt.cpp), seeded with 1 by
// rng::srand_4F7A40. The first values are those of the original's C runtime.
#include <doctest.h>

#include "core/rng.hpp"

TEST_CASE("rand() is MSVC's")
{
    Rng r;
    r.Srand();
    const int expected[] = {41, 18467, 6334, 26500, 19169, 15724, 11478, 29358, 26962, 24464};
    for (int v : expected)
    {
        CHECK(r.Rand() == v);
    }
}

TEST_CASE("get_int and get_uint8 take rand() modulo the limit")
{
    Rng r;
    r.Srand();
    const s16 max16 = 100;
    CHECK(r.GetInt(max16) == 41);
    CHECK(r.GetInt(max16) == 18467 % 100);
    const u8 max8 = 7;
    CHECK(r.GetUint8(max8) == 6334 % 7);
    const s16 zero = 0;
    CHECK(r.GetInt(zero) == 0); // and draws nothing
    CHECK(r.Rand() == 26500);
}

TEST_CASE("the cycle counter counts updates")
{
    Rng r;
    r.Srand();
    r.AdvanceCycle();
    r.AdvanceCycle();
    CHECK(r.GetCurRng() == 2);
}
