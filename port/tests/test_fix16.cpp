// Fix16: the game's 18.14 fixed-point number. Values are mValue / 16384.
#include <doctest.h>

#include "core/fix16.hpp"

TEST_CASE("Fix16 converts from and to integers")
{
    CHECK(Fix16(1).value_ == 0x4000);
    CHECK(Fix16(-3).value_ == -3 * 0x4000);
    CHECK(Fix16(5).ToInt() == 5);
    CHECK(Fix16(0x6000, 0).ToInt() == 1); // 1.5 rounds towards minus infinity
    CHECK(Fix16(-0x6000, 0).ToInt() == -2);
    Fix16 f;
    f.FromInt(7);
    CHECK(f.value_ == 7 * 0x4000);
}

TEST_CASE("Fix16 arithmetic")
{
    CHECK((Fix16(2) + Fix16(3)).value_ == Fix16(5).value_);
    CHECK((Fix16(2) - Fix16(3)).value_ == Fix16(-1).value_);
    CHECK((Fix16(2) * Fix16(3)).value_ == Fix16(6).value_);
    CHECK((Fix16(0x2000, 0) * Fix16(0x2000, 0)).value_ == 0x1000); // 0.5 * 0.5
    CHECK((Fix16(3) * 4).value_ == Fix16(12).value_);
    CHECK((-Fix16(3)).value_ == Fix16(-3).value_);

    Fix16 q(7);
    q /= Fix16(2);
    CHECK(q.value_ == 0xE000); // 3.5

    Fix16 m(3);
    m *= Fix16(0x2000, 0);
    CHECK(m.value_ == 0x6000);
}

TEST_CASE("Fix16 multiplication truncates towards minus infinity")
{
    // The product is shifted right, as the original's (64-bit product) >> 14
    CHECK((Fix16(1, 0) * Fix16(1, 0)).value_ == 0);
    CHECK((Fix16(-1, 0) * Fix16(1, 0)).value_ == -1);
}

TEST_CASE("Fix16 fraction, rounding and absolute value")
{
    const Fix16 v(0x6123, 0);
    CHECK(v.GetFracValue().value_ == 0x2123);
    CHECK(v.GetRoundValue().value_ == 0x4000);
    CHECK(Fix16::Abs(Fix16(-2)).value_ == Fix16(2).value_);
    CHECK(Fix16::Max(Fix16(1), Fix16(2)).value_ == Fix16(2).value_);
}

TEST_CASE("Fix16 square root")
{
    CHECK(Fix16::SquareRoot(Fix16(4)).value_ == Fix16(2).value_);
    CHECK(Fix16::SquareRoot(Fix16(9)).value_ == Fix16(3).value_);
    CHECK(Fix16::SquareRoot(Fix16(0)).value_ == 0);
    CHECK(Fix16::SquareRoot(Fix16(2)).value_ == 23170); // sqrt(2) * 16384 = 23170.47
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
