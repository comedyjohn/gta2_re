// The game's random numbers: MSVC's rand() (port/platform/crt.cpp), seeded with 1 by
// rng::srand_4F7A40. The first values are those of the original's C runtime.
#include <doctest.h>

#include "rng.hpp"

TEST_CASE("rand() is MSVC's")
{
    Rng r;
    r.srand_4F7A40();
    const int expected[] = {41, 18467, 6334, 26500, 19169, 15724, 11478, 29358, 26962, 24464};
    for (int v : expected)
    {
        CHECK(r.rand_4F7C00() == v);
    }
}

TEST_CASE("get_int and get_uint8 take rand() modulo the limit")
{
    Rng r;
    r.srand_4F7A40();
    const s16 max16 = 100;
    CHECK(r.get_int_4F7AE0(max16) == 41);
    CHECK(r.get_int_4F7AE0(max16) == 18467 % 100);
    const u8 max8 = 7;
    CHECK(r.get_uint8_4F7B70(max8) == 6334 % 7);
    const s16 zero = 0;
    CHECK(r.get_int_4F7AE0(zero) == 0); // and draws nothing
    CHECK(r.rand_4F7C00() == 26500);
}

TEST_CASE("the cycle counter counts updates")
{
    Rng r;
    r.srand_4F7A40();
    r.AdvanceCycle_48B900();
    r.AdvanceCycle_48B900();
    CHECK(r.get_cur_rng_41CFE0() == 2);
}
