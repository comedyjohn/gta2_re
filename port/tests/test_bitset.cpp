#include <doctest.h>

#include "BitSet32.hpp"

TEST_CASE("BitSet32 sets, clears and tests bits")
{
    BitSet32 b{};
    b.set_bit(0);
    b.set_bit(31);
    CHECK(b.check_bit(0));
    CHECK(b.check_bit(31));
    CHECK_FALSE(b.check_bit(5));
    CHECK(b.GetRaw_4A5190() == 0x80000001u);
    b.clear_bit(0);
    CHECK_FALSE(b.check_bit(0));
    CHECK(b.mask_bit(31));
}
