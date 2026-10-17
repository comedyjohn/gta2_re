// The trigonometry tables, the square root and arctangent, and rand(): what every machine of a
// network game must compute alike (gta2 --simcheck). The hashes are those of the GCC 32-bit build,
// which matches the 64-bit, Clang and ARM builds.
#include <doctest.h>

#include "simcheck.h"

#include "ang16.hpp"
#include "fix16.hpp"

TEST_CASE("simcheck hashes")
{
    const SimcheckHashes h = port_simcheck_hashes();
    CHECK(h.trigonometry == 0xe0d0be9e58500fa6ULL);
    CHECK(h.random == 0xe7012a80d9cc4025ULL);
    CHECK(h.maths == 0x92fd71d7f5ee31efULL);
}

TEST_CASE("sine and cosine tables")
{
    Init_trigonometry_tables();
    // 1440 steps a turn: 360 is 90 degrees
    CHECK(Ang16::sine_40F500(Ang16(0)).mValue == 0);
    CHECK(Ang16::sine_40F500(Ang16(360)).mValue == 0x4000);
    CHECK(Ang16::cosine_40F520(Ang16(0)).mValue == 0x4000);
    CHECK(Ang16::cosine_40F520(Ang16(720)).mValue == -0x4000);

    Fix16 x;
    Fix16 y;
    Ang16::PolarToCartesian_41FC20(Ang16(360), Fix16(2), x, y);
    CHECK(x.mValue == Fix16(2).mValue);
    CHECK(y.mValue == 0);
}

TEST_CASE("angles wrap to 0..1439")
{
    Ang16 a(1400);
    a += Ang16(100);
    CHECK(a.rValue == 60);
    Ang16 b(10);
    b -= Ang16(20);
    CHECK(b.rValue == 1430);
}
