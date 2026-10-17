// gta2 --simcheck: what a network game needs every machine to compute alike, hashed. Every machine
// runs the whole simulation (lockstep), so the numbers it draws and the tables its maths uses must be
// the same on a 32-bit PC, a 64-bit one and an ARM handheld. port/tools/nettest.sh compares the
// output of builds for different systems; it must be identical.
#include "simcheck.h"

#include "core/ang16.hpp"
#include "core/fix16.hpp"

#include <stdio.h>

EXTERN_GLOBAL_ARRAY(Fix16, gTanTable, 1440);

namespace
{
struct Hash
{
    uint64_t h = 1469598103934665603ULL;
    void Add(uint32_t v)
    {
        for (int i = 0; i < 4; i++)
        {
            h ^= (v >> (i * 8)) & 0xFF;
            h *= 1099511628211ULL;
        }
    }
};
} // namespace

SimcheckHashes port_simcheck_hashes()
{
    InitTrigonometryTables();

    Hash trig;
    for (int i = 0; i < 1440; i++)
    {
        trig.Add(gSinTable[i].mValue);
        trig.Add(gCosTable[i].mValue);
        trig.Add(gTanTable[i].mValue);
    }

    // rng::get_int_4F7AE0 and friends draw from rand(), seeded with 1 (rng::srand_4F7A40)
    Hash random;
    srand(1);
    for (int i = 0; i < 100000; i++)
    {
        random.Add(rand());
    }

    // The two places floating point meets the simulation: square roots and arctangents
    Hash maths;
    for (s32 v = 0; v < (1 << 24); v += 37)
    {
        Fix16 f(v, 0);
        maths.Add(Fix16::SquareRoot(f).mValue);
    }
    for (s32 y = -40000; y <= 40000; y += 997)
    {
        for (s32 x = -40000; x <= 40000; x += 1009)
        {
            maths.Add(Fix16::Atan2Fixed(Fix16(y, 0), Fix16(x, 0)).rValue);
        }
    }

    return {trig.h, random.h, maths.h};
}

int port_simcheck()
{
    const SimcheckHashes h = port_simcheck_hashes();
    printf("sin(90) %d, cos(180) %d\n", gSinTable[360].mValue, gCosTable[720].mValue);
    printf("trigonometry %016llx\n", static_cast<unsigned long long>(h.trigonometry));
    printf("random %016llx\n", static_cast<unsigned long long>(h.random));
    printf("maths %016llx\n", static_cast<unsigned long long>(h.maths));
    return 0;
}
