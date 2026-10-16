// Fix16: the game's 18.14 fixed-point number. Values are mValue / 16384.
#include <doctest.h>

#include "fix16.hpp"

TEST_CASE("Fix16 converts from and to integers")
{
    CHECK(Fix16(1).mValue == 0x4000);
    CHECK(Fix16(-3).mValue == -3 * 0x4000);
    CHECK(Fix16(5).ToInt() == 5);
    CHECK(Fix16(0x6000, 0).ToInt() == 1); // 1.5 rounds towards minus infinity
    CHECK(Fix16(-0x6000, 0).ToInt() == -2);
    Fix16 f;
    f.FromInt(7);
    CHECK(f.mValue == 7 * 0x4000);
}

TEST_CASE("Fix16 arithmetic")
{
    CHECK((Fix16(2) + Fix16(3)).mValue == Fix16(5).mValue);
    CHECK((Fix16(2) - Fix16(3)).mValue == Fix16(-1).mValue);
    CHECK((Fix16(2) * Fix16(3)).mValue == Fix16(6).mValue);
    CHECK((Fix16(0x2000, 0) * Fix16(0x2000, 0)).mValue == 0x1000); // 0.5 * 0.5
    CHECK((Fix16(3) * 4).mValue == Fix16(12).mValue);
    CHECK((-Fix16(3)).mValue == Fix16(-3).mValue);

    Fix16 q(7);
    q /= Fix16(2);
    CHECK(q.mValue == 0xE000); // 3.5

    Fix16 m(3);
    m *= Fix16(0x2000, 0);
    CHECK(m.mValue == 0x6000);
}

TEST_CASE("Fix16 multiplication truncates towards minus infinity")
{
    // The product is shifted right, as the original's (64-bit product) >> 14
    CHECK((Fix16(1, 0) * Fix16(1, 0)).mValue == 0);
    CHECK((Fix16(-1, 0) * Fix16(1, 0)).mValue == -1);
}

TEST_CASE("Fix16 fraction, rounding and absolute value")
{
    const Fix16 v(0x6123, 0);
    CHECK(v.GetFracValue().mValue == 0x2123);
    CHECK(v.GetRoundValue().mValue == 0x4000);
    CHECK(Fix16::Abs(Fix16(-2)).mValue == Fix16(2).mValue);
    CHECK(Fix16::Max_41E130(Fix16(1), Fix16(2)).mValue == Fix16(2).mValue);
}

TEST_CASE("Fix16 square root")
{
    CHECK(Fix16::SquareRoot_436A70(Fix16(4)).mValue == Fix16(2).mValue);
    CHECK(Fix16::SquareRoot_436A70(Fix16(9)).mValue == Fix16(3).mValue);
    CHECK(Fix16::SquareRoot_436A70(Fix16(0)).mValue == 0);
    CHECK(Fix16::SquareRoot_436A70(Fix16(2)).mValue == 23170); // sqrt(2) * 16384 = 23170.47
}

TEST_CASE("Fix16 comparisons")
{
    CHECK(Fix16(1) < Fix16(2));
    CHECK(Fix16(2) > Fix16(1));
    CHECK(Fix16(2) >= Fix16(2));
    CHECK(Fix16(2) <= Fix16(2));
    CHECK(Fix16(2) == Fix16(2));
    CHECK(Fix16(2) != Fix16(3));
}
