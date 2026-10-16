#pragma once

#include "Function.hpp"
#include "ang16.hpp"
#include "fix16.hpp"

EXTERN_GLOBAL(Fix16, gFix16_6777CC);
EXTERN_GLOBAL(Fix16, kFP16Zero_6FE20C);
EXTERN_GLOBAL(Fix16, kFP16One256th_6FE07C);
EXTERN_GLOBAL(Fix16, kFpZero_6F77C0);

// GetLength_41E260 compares against a zero constant that each TU of the original has its own copy of
// (a header static, like 9.6f's single 0x5E3DC8 in vec_len 0x41E260). A TU whose copy isn't
// gFix16_6777CC defines FIX16_POINT_ZERO to it before its first include.
#ifndef FIX16_POINT_ZERO
    #define FIX16_POINT_ZERO gFix16_6777CC
#endif
EXTERN_GLOBAL(Fix16, kFpZero_6F610C);
EXTERN_GLOBAL(Fix16, kF16Zero_677B90);
EXTERN_GLOBAL(Fix16, dword_706EB8);
EXTERN_GLOBAL(Fix16, kFP16Zero_6FD9E4);
EXTERN_GLOBAL(Fix16, kZero_676818);
EXTERN_GLOBAL(Fix16, kFpZero_7064C0);
EXTERN_GLOBAL(Fix16, kZero_679E70);

class Fix16_Point;

// A point without a destructor. 10.5 has points that don't take part in EH unwinding: struct members
// (~Sprite is a bare jmp, the Char_B4 ctor's EH states leave out field_98_velocity_vector), a by-value
// parameter the caller copies as two dwords (sub_5DE910) and a few locals (CarPhysics_B0 0x55F280,
// 0x55F800, Camera_0xBC::WorldToScreen_40CFC0's result). Everything else is a Fix16_Point. It is a
// separate type, not a base of Fix16_Point: 10.5 charges Fix16_Point's ctor as one size-42 inline
// site at the top level, which a derived class's ctor (base ctor nested in it) doesn't reproduce.
// 9.6f doesn't tell the two apart.
struct Fix16_Point_POD
{
    Fix16_Point_POD() = default;

    Fix16_Point_POD(Fix16& a1, Fix16& a2)
    {
        x = a1;
        y = a2;
    }

    void SetXY_432860(const Fix16& a2, const Fix16& a3)
    {
        this->x = a2;
        this->y = a3;
    }

    inline void SetFromPolar_41E210(const Fix16& radius, const Ang16& angle)
    {
        x = Ang16::sine_40F500(angle) * radius;
        y = Ang16::cosine_40F520(angle) * radius;
    }

    // Matching impl at RotateVelocity_562C20
    inline void RotateByAngle_40F6B0(const Ang16& angle)
    {
        Fix16 sin = Ang16::sine_40F500(angle);
        Fix16 cos = Ang16::cosine_40F520(angle);

        Fix16 x_old = x;

        x = (x * cos) + (y * sin);
        y = ((-x_old) * sin) + (y * cos);
    }

    Fix16_Point_POD& operator+=(Fix16_Point& other);

    // Same layout: a POD passed where a Fix16_Point is expected (a free inline site, like the other way)
    operator Fix16_Point&()
    {
        return *reinterpret_cast<Fix16_Point*>(this);
    }

    Fix16 x;
    Fix16 y;
};

class Fix16_Point
{
  public:
    void SetXY_432860(const Fix16& a2, const Fix16& a3)
    {
        this->x = a2;
        this->y = a3;
    }

    // Reads the TU's FIX16_POINT_ZERO copy (Weapon_30::throwable_5DDFC0 compares with 0x706EB8)
    inline bool IsNull_420360() const
    {
        return x == FIX16_POINT_ZERO && y == FIX16_POINT_ZERO;
    }

    // For some reason uses another constant
    inline bool IsNull() const
    {
        return x == kFP16Zero_6FE20C && y == kFP16Zero_6FE20C;
    }

    // 9.6f 0x49E450
    inline bool HasZeroComponent_49E450() const
    {
        return x == kFP16Zero_6FE20C || y == kFP16Zero_6FE20C;
    }

    void ApplyDeadZone_49E3C0()
    {
        Fix16 total = (Fix16::Abs(x) + Fix16::Abs(y));
        if (total < kFP16One256th_6FE07C)
        {
            x = kFP16Zero_6FE20C;
            y = kFP16Zero_6FE20C;
        }
    }

    // But also 0x40ACD0 non inlined in 10.5
    Ang16 atan2_40F790() const
    {
        return Fix16::atan2_fixed_405320(y, x);
    }

    Fix16 GetLength_453590();

    // 9.6f 0x41E260; the out-of-line copy is GetLength_453590. The nested if/else (front-end size 162, the
    // else-if chain is 160) gives the original's cut-offs in Crane_15C::ComputeHookPolar_47F6C0 with
    // SquareRoot at 48; matched functions allow 159..162.
    inline Fix16 GetLength_41E260() const
    {
        if (x == FIX16_POINT_ZERO)
        {
            return Fix16::Abs(y);
        }
        else
        {
            if (y == FIX16_POINT_ZERO)
            {
                return Fix16::Abs(x);
            }
            else
            {
                return Fix16::SquareRoot(x * x + y * y);
            }
        }
    }

    inline void SetFromPolar_41E210(const Fix16& radius, const Ang16& angle)
    {
        x = Ang16::sine_40F500(angle) * radius;
        y = Ang16::cosine_40F520(angle) * radius;
    }

    // Matching impl at RotateVelocity_562C20
    inline void RotateByAngle_40F6B0(const Ang16& angle)
    {
        Fix16 sin = Ang16::sine_40F500(angle);
        Fix16 cos = Ang16::cosine_40F520(angle);

        Fix16 x_old = x;

        x = (x * cos) + (y * sin);
        y = ((-x_old) * sin) + (y * cos);
    }

    void FromPolar_41E210(const Fix16& radius, const Ang16& angle)
    {

        x = radius * Ang16::sine_40F500(angle);
        y = radius * Ang16::cosine_40F520(angle);
    }

    Fix16_Point& operator+=(const Fix16_Point& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    // FUNCTION: 96f 0x4828c0
    Fix16_Point& operator-=(Fix16_Point& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Operator* for Fix16 ?
    Fix16_Point& MultiplyByFix16_49E3A0(const Fix16& factor)
    {
        x *= factor;
        y *= factor;
        return *this;
    }

    Fix16_Point Multiply_438FE0(const Fix16& in) const;
    Fix16_Point Divide_442CB0(const Fix16& in);

    // Divide_442CB0 as a nothrow inline (see DivideInl_55F9E0; CarPhysics_B0::HandleObjectCollision_5606C0,
    // Car_BC::ApplyExplosionImpulse_443710)
    inline Fix16_Point DivideInl_442CB0(Fix16& in) throw()
    {
        return Fix16_Point(x / in, y / in);
    }

    // Multiply_438FE0 as a nothrow inline (see DivideInl_55F9E0; Car_BC::TryHitchTrailer_442810)
    inline Fix16_Point MultiplyInl_438FE0(Fix16& in) throw()
    {
        return Fix16_Point(x * in, y * in);
    }

    // Out-of-line copies emitted in Weapon_30.cpp (used by sub_5DE910).
    Fix16_Point& AddAssign_5E40C0(const Fix16_Point& other);
    Fix16_Point& DivAssign_5E40E0(const Fix16& v);
    Fix16 MaxAbs_5E4140();

    // 9.6f 0x48A270 (the inline of MaxAbs_5E4140)
    inline Fix16 MaxAbs_48A270()
    {
        Fix16 ax = Fix16::Abs_436A50(x);
        Fix16 ay = Fix16::Abs_436A50(y);
        if (ax > ay)
        {
            return ax;
        }
        return ay;
    }

    // 9.6f 0x48A250: both components through Fix16::DivideAssign (10.5 0x539F90)
    inline void DivideAssign_48A250(const Fix16& d)
    {
        x.DivideAssign_539F90(d);
        y.DivideAssign_539F90(d);
    }

    // FUNCTION: 96f 0x41e1e0
    void reset()
    {
        x = Fix16(0);
        y = Fix16(0);
    }

    // Both inlined and exists as a function... some strange array init behaviour??
    ~Fix16_Point() = default;

    // It needs to be in the header
    // 0x563970

    Fix16_Point() = default;

    // No user-defined copy ctor: 9.6f copies points with plain movs (no call even at /Ob0), and the
    // implicit one does not use up VC6's inline budget (SpawnCabAndTrailerHelper_408370 needs that).

    // 9.6f 0x401D20
    Fix16_Point(const Fix16& a1, const Fix16& a2) : x(a1), y(a2)
    {
    }

    void ClampTowardsZero_49E480(const Fix16_Point& limit)
    {
        if (x >= kFP16Zero_6FE20C)
        {
            if (x > limit.x)
            {
                x = limit.x;
            }
        }
        else if (x < limit.x)
        {
            x = limit.x;
        }

        if (y >= kFP16Zero_6FE20C)
        {
            if (y > limit.y)
            {
                y = limit.y;
            }
        }
        else if (y < limit.y)
        {
            y = limit.y;
        }
    }

    // 0x40AC50
    Fix16_Point operator+(const Fix16_Point& in)
    {
        return Fix16_Point(x + in.x, y + in.y);
    }

    // 0x40AC80
    Fix16_Point operator-(const Fix16_Point& rhs) const
    {
        return Fix16_Point(x - rhs.x, y - rhs.y);
    }

    // The out-of-line copy of the inline operator-. Called by name where the original keeps an EH state
    // for temporaries around the call (Crane_15C::HookPickupCar_47EF80, ComputeHookPolar_47F6C0); the
    // inline operator- called out of line gets none (CarPhysics_B0::HandleCarCollision_55FF20)
    Fix16_Point Sub_40AC80(const Fix16_Point& rhs) const;

    // operator+/operator- with their bodies defined at the end of Car_BC.cpp (HandleCarHitByObject_43F130)
    inline Fix16_Point AddLate_40AC50(const Fix16_Point& in);
    inline Fix16_Point SubLate_40AC80(const Fix16_Point& rhs);

    // Out of line operator+ (CarPhysics_B0::SpawnSkidSegment_55D200)
    Fix16_Point Add_40AC50(const Fix16_Point& in);

    // operator+ 0x40AC50 as a nothrow inline that VC6 still calls out of line: no EH state for the
    // temporaries alive across the call (Car_BC::TryHitchTrailer_442810)
    inline Fix16_Point AddInl_40AC50(const Fix16_Point& in) throw()
    {
        return Fix16_Point(x + in.x, y + in.y);
    }

    // Out of line unary minus (Object_2C::ResolveCollisionWithPed_5229B0)
    Fix16_Point Negate_40ACB0() const;

    Fix16_Point operator*(const Fix16& in)
    {
        return Fix16_Point(x * in, y * in);
    }

    Fix16_Point operator-() const
    {
        return Fix16_Point(-x, -y);
    }

    Fix16_Point MultBy_442C80(const s32& factor);

    // 10.0 0x442CB0
    Fix16_Point operator/(const Fix16& in) const;

    Fix16_Point NormalizeSafe_442AD0();

    Ang16 atan2_40ACD0();

    Fix16_Point Rotate90CCW_5605E0();

    void clear_41E1E0()
    {
        x = 0;
        y = 0;
    }

    Fix16_Point operator/(const s32& a3);

    // operator/ 0x55F9E0 as a nothrow inline that VC6 still calls out of line: the original sets no EH
    // state for the temporaries that live across this call (CarPhysics_B0::HandleCarCollision_55FF20,
    // HandleObjectCollision_5606C0). throw() on the real operator/ changes 6 matched functions.
    inline Fix16_Point DivideInl_55F9E0(const s32& a3) throw()
    {
        return Fix16_Point(x / a3, y / a3);
    }

    // Same layout: a point sliced into a Fix16_Point_POD (one free inline site, see sub_5DF270)
    operator Fix16_Point_POD&()
    {
        return *reinterpret_cast<Fix16_Point_POD*>(this);
    }

    Fix16 x;
    Fix16 y;
};

inline Fix16_Point_POD& Fix16_Point_POD::operator+=(Fix16_Point& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

struct Fix16_Vec
{
    Fix16 x, y, z;
};
