#include <doctest.h>

#include "core/bit_set32.hpp"

TEST_CASE("BitSet32 sets, clears and tests bits")
{
    BitSet32 b {};
    b.SetBit(0);
    b.SetBit(31);
    CHECK(b.CheckBit(0));
    CHECK(b.CheckBit(31));
    CHECK_FALSE(b.CheckBit(5));
    CHECK(b.GetRaw() == 0x80000001U);
    b.ClearBit(0);
    CHECK_FALSE(b.CheckBit(0));
    CHECK(b.MaskBit(31));
}
