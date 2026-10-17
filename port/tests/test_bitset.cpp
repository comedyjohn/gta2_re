#include <doctest.h>

#include "BitSet32.hpp"

TEST_CASE("BitSet32 sets, clears and tests bits")
{
    BitSet32 b {};
    b.SetBit(0);
    b.SetBit(31);
    CHECK(b.check_bit(0));
    CHECK(b.check_bit(31));
    CHECK_FALSE(b.check_bit(5));
    CHECK(b.GetRaw() == 0x80000001U);
    b.ClearBit(0);
    CHECK_FALSE(b.check_bit(0));
    CHECK(b.mask_bit(31));
}
