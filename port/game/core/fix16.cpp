#include "core/fix16.hpp"
#include "core/ang16.hpp"
#include "core/function.hpp"
#include <cmath>

DEFINE_GLOBAL_ARRAY(Fix16, gSinTable, 1440, 0x667A80);
DEFINE_GLOBAL_ARRAY(Fix16, gCosTable, 1440, 0x669260);

// TODO: hmm, shouldn't this be 360 entries ??
DEFINE_GLOBAL_ARRAY(Fix16, gTanTable, 1440, 0x6663C8);
DEFINE_GLOBAL_INIT(Fix16, kFixZero, Fix16(0), 0x6691B0);
DEFINE_GLOBAL_INIT(Ang16, kAngZero, Ang16(0), 0x66A920);
DEFINE_GLOBAL_INIT(Ang16, kAng180, Ang16(720), 0x669156);
DEFINE_GLOBAL_INIT(Ang16, kAng90, Ang16(360), 0x667A7C);
DEFINE_GLOBAL_INIT(Ang16, kAng270, Ang16(1080), 0x66916C);

// 0x408660
Fix16 Fix16::Add(const Fix16& rhs) const throw()
{
    s32 value = value_ + rhs.value_;
    return Fix16(value, 0);
}

// 0x408680
Fix16 Fix16::Multiply(const Fix16& in) const throw()
{
    s32 value = static_cast<s32>((value_ * static_cast<__int64>(in.value_)) >> 14);
    return Fix16(value, 0);
}

// 0x436A00
Fix16 Fix16::Subtract(const Fix16& in) const
{
    s32 value = value_ - in.value_;
    return Fix16(value, 0);
}

// 0x436A20
Fix16 Fix16::Divide(const Fix16& in) const
{
    s32 value = static_cast<s32>((static_cast<__int64>(value_) << 14) / in.value_);
    return Fix16(value, 0);
}

// 0x451670
s32 Fix16::IsLess(const Fix16& other) const
{
    return value_ < other.value_;
}

// 0x451690
s32 Fix16::IsGreater(const Fix16& other) const
{
    return value_ > other.value_;
}

// 0x539F90
Fix16& Fix16::DivideAssign(const Fix16& rhs)
{
    value_ = static_cast<s32>((static_cast<__int64>(value_) << 14) / rhs.value_);
    return *this;
}

// Out-of-line copy of operator/(const s32&) (20 bytes, called by Particle_8::EmitImpactParticles_53FE40)
// 0x53E860
Fix16 Fix16::DivideInt(const s32& in) const
{
    s32 value = value_ / in;
    return Fix16(value, 0);
}

// 0x561DB0
Fix16 Fix16::MultiplyInt(const s32& in) const
{
    s32 value = value_ * in;
    return Fix16(value, 0);
}

// 0x562430
Fix16& Fix16::MultiplyAssign(const Fix16& rhs)
{
    value_ = static_cast<s32>((value_ * static_cast<__int64>(rhs.value_)) >> 14);
    return *this;
}

// 0x4086A0
Fix16 Fix16::Negated() const throw()
{
    return Fix16(-value_, 0);
}

/*
// 0x436A20
Fix16 Fix16::operator/(const Fix16& in)
{
    s32 value = ((__int64)mValue << 14) / in.mValue;
    return Fix16(value, 0);
}
*/

// 10.5 https://decomp.me/scratch/7a41K
// 9.6f https://decomp.me/scratch/ZkUbq
// 0x405320
Ang16 Fix16::Atan2Fixed(const Fix16& x, const Fix16& y)
{
    Ang16 table_angle;
    if (y == kFixZero)
    {
        if (x >= kFixZero)
        {
            return kAngZero;
        }
        else
        {
            return kAng180;
        }
    }
    else if (x == kFixZero)
    {
        if (y > kFixZero)
        {
            return kAng90;
        }
        else
        {
            return kAng270;
        }
    }
    else
    {
        table_angle = ArcTanLookup(Fix16::Abs(x / y));

        if (x > kFixZero)
        {
            if (y > kFixZero)
            {
                return kAng90 - table_angle;
            }
            else
            {
                if (table_angle == kAng90)
                {
                    return kAngZero;
                }
                else
                {
                    return kAng270 + table_angle;
                }
            }
        }
        else
        {
            if (y > kFixZero)
            {
                return kAng90 + table_angle;
            }
            else
            {
                return kAng270 - table_angle;
            }
        }
    }
}

// 0x438FB0
bool IntervalIntersectsRange(const Fix16& interval_start, const Fix16& interval_end, const Fix16& range_min, const Fix16& range_max)
{
    if (interval_start < range_min)
    {
        return interval_end >= range_min ? true : false;
    }
    else
    {
        return interval_start <= range_max ? true : false;
    }
}

EXTERN_GLOBAL(Fix16, kAngFix16FullCircle);
EXTERN_GLOBAL(Fix16, kAngFix16HalfCircle);

// Turns the angle `cur` toward `*pTarget` by at most `*pSpeed`, the short way round, and wraps the
// result into [0, 2pi). Called by Trailer::UpdateTrailerAlignment_407CE0.
// 0x405DA0
Fix16 TurnAngleToward(Fix16 cur, Fix16* target, Fix16* speed)
{
    if (*target - cur > kAngFix16HalfCircle)
    {
        cur += kAngFix16FullCircle;
    }
    else if (*target - cur < -kAngFix16HalfCircle)
    {
        cur -= kAngFix16FullCircle;
    }

    Fix16 diff = *target - cur;
    if (diff > kFixZero)
    {
        if (diff > *speed)
        {
            diff = *speed;
        }
    }
    else if (diff < kFixZero)
    {
        if (diff < -*speed)
        {
            diff = -*speed;
        }
    }

    Fix16 result = diff + cur;
    for (; result < kFixZero; result += kAngFix16FullCircle)
    {
        ;
    }
    for (; result >= kAngFix16FullCircle; result -= kAngFix16FullCircle)
    {
        ;
    }
    return result;
}

DEFINE_GLOBAL_INIT(Fix16, kAngleNearTolerance, Fix16(0x11C, 0), 0x66A924);
DEFINE_GLOBAL_INIT(Fix16, kHitchAngleWindow, Fix16(0x63D8, 0), 0x669140);
DEFINE_GLOBAL_INIT(Fix16, kHitchAngleWrapOffset, Fix16(0x12B88, 0), 0x6691FC);

// Is `*a` within dword_66A924 of `*b`, directly or one turn (kAngFix16FullCircle_66A8E4) either way.
// 0x405E20
s32 IsAngleNear(Fix16* a, Fix16* b)
{
    if ((*a > *b - kAngleNearTolerance && *a < *b + kAngleNearTolerance) ||
        (*a > *b - kAngFix16FullCircle - kAngleNearTolerance && *a < *b - kAngFix16FullCircle + kAngleNearTolerance) ||
        (*a > *b - kAngleNearTolerance + kAngFix16FullCircle && *a < *b + kAngFix16FullCircle + kAngleNearTolerance))
    {
        return 1;
    }
    return 0;
}

// Clamps the angle `*pCur` into the window of +-dword_669140 around `*pTarget`, allowing for the
// wrap at 0 / 2pi, and returns whether it ends up on either edge. Called by
// Trailer::UpdateTrailerAlignment_407CE0.
// 0x405E80
s32 ClampAngleToWindow(Fix16* target, Fix16* cur)
{
    Fix16 lo;
    Fix16 hi;
    if (*target < kHitchAngleWindow)
    {
        lo = kHitchAngleWrapOffset + *target;
        hi = *target + kHitchAngleWindow;
        if (*cur > lo - kHitchAngleWindow && *cur < lo)
        {
            *cur = lo;
        }
        else if (*cur > hi && *cur < lo)
        {
            *cur = hi;
        }
    }
    else if (*target < kAngFix16HalfCircle)
    {
        lo = *target - kHitchAngleWindow;
        hi = *target + kHitchAngleWindow;
        if (*cur < lo || *cur > hi + kHitchAngleWindow)
        {
            *cur = lo;
        }
        else if (*cur > hi)
        {
            *cur = hi;
        }
    }
    else if (*target < kHitchAngleWrapOffset)
    {
        lo = *target - kHitchAngleWindow;
        hi = *target + kHitchAngleWindow;
        if (*cur < hi - kHitchAngleWrapOffset || *cur > hi)
        {
            *cur = hi;
        }
        else if (*cur < lo)
        {
            *cur = lo;
        }
    }
    else
    {
        lo = *target - kHitchAngleWindow;
        hi = *target - kHitchAngleWrapOffset;
        if (*cur > hi && *cur < hi + kHitchAngleWindow)
        {
            *cur = hi;
        }
        else if (*cur < lo && *cur > hi)
        {
            *cur = lo;
        }
    }

    if (static_cast<u8>(IsAngleNear(cur, &lo)) || static_cast<u8>(IsAngleNear(cur, &hi)))
    {
        return 1;
    }
    return 0;
}

// The original is a CRT init func (called from the CRT init table), here Init_trigonometry_tables
// calls it. The constants are pi and 1/720: 1440 steps of the full circle.
// 0x4052D0
void ArcTanTableInit()
{
    s32 arg = 0;
    Fix16* tan_entry = gTanTable;
    for (s32 i = 1440; i != 0; i--)
    {
        f64 radians = arg * 3.141592654;
        const f64 value = tan(radians * 0.001388888888888889) * 16384.0;
        // tan(90) and tan(270) don't fit a Fix16. The original's conversion (x87 fistp) gives
        // 0x80000000 for them, as x86's SSE one does; C++ leaves it undefined, and ARM saturates.
        *tan_entry = Fix16(value >= -2147483648.0 && value < 2147483648.0 ? static_cast<s32>(value) : INT32_MIN, 0);
        arg++;
        tan_entry++;
    }
}

// 9.6f: 0x40E810
// 0x405500
Ang16 ArcTanLookup(const Fix16& target_tan)
{
    s16 low = 0;
    s16 high = 360 - 1;

    while (true)
    {
        s16 mid = (low + high) >> 1;
        Fix16 tan_at_mid = gTanTable[mid];

        // Binary search: go left if target < tanAtMid,
        // go right if target > tanAtMid,
        // exact match if equal
        if (target_tan < tan_at_mid)
        {
            high = mid - 1;
        }
        else if (target_tan > tan_at_mid)
        {
            low = mid + 1;
        }
        else
        {
            return Ang16(mid, 0);
        }

        if (low > high)
        {
            // Lower bound case
            if (mid == (0 + 1))
            {
                if (target_tan < gTanTable[0 + 1])
                {
                    mid = 0;
                }
            }
            // Upper bound case
            else if (mid == (360 - 1))
            {
                if (target_tan > gTanTable[360 - 1])
                {
                    mid = 360;
                }
            }
            return Ang16(mid, 0);
        }
    }
}

// 0x5A57E0
void FindMinMax(Fix16& min_out, Fix16& max_out, const Fix16& a, const Fix16& b, const Fix16& c, const Fix16& d)
{
    min_out = a;
    max_out = a;

    // b
    if (b < min_out)
    {
        min_out = b;
    }
    else if (b > max_out)
    {
        max_out = b;
    }

    // c
    if (c < min_out)
    {
        min_out = c;
    }
    else if (c > max_out)
    {
        max_out = c;
    }

    // d
    if (d < min_out)
    {
        min_out = d;
    }
    else if (d > max_out)
    {
        max_out = d;
    }
}

void InitTrigonometryTables()
{
    s16 arg = 0;
    for (u32 idx = 0; idx < GTA2_COUNTOF(gSinTable); idx++, arg++)
    {
        gSinTable[idx] = Fix16(sin((static_cast<f64>(arg) / 1440.0) * 2 * 3.141592654));
    }
    arg = 0;
    for (u32 entry = 0; entry < GTA2_COUNTOF(gCosTable); entry++, arg++)
    {
        gCosTable[entry] = Fix16(cos((static_cast<f64>(arg) / 1440.0) * 2 * 3.141592654));
    }
    ArcTanTableInit();
    printf("Sine, cosine and tangent tables initialized!\n");
}
