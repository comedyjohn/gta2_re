#pragma once

#include "core/ang16.hpp"
#include "core/fix16.hpp"
#include "core/function.hpp"

EXTERN_GLOBAL(Fix16, gFix16_6777CC);
EXTERN_GLOBAL(Fix16, kFP16One256th_6FE07C);
EXTERN_GLOBAL(Fix16, kFpZero_6F77C0);

// GetLength_41E260 compares against a zero constant that each TU of the original has its own copy of
// (a header static, like 9.6f's single 0x5E3DC8 in vec_len 0x41E260). A TU whose copy isn't
// gFix16_6777CC defines FIX16_POINT_ZERO to it before its first include.
#ifndef FIX16_POINT_ZERO
    #define FIX16_POINT_ZERO gFix16_6777CC
#endif

class Fix16Point;

// A point without a destructor. 10.5 has points that don't take part in EH unwinding: struct members
// (~Sprite is a bare jmp, the Char_B4 ctor's EH states leave out field_98_velocity_vector), a by-value
// parameter the caller copies as two dwords (sub_5DE910) and a few locals (CarPhysics_B0 0x55F280,
// 0x55F800, Camera_0xBC::WorldToScreen_40CFC0's result). Everything else is a Fix16_Point. It is a
// separate type, not a base of Fix16_Point: 10.5 charges Fix16_Point's ctor as one size-42 inline
// site at the top level, which a derived class's ctor (base ctor nested in it) doesn't reproduce.
// 9.6f doesn't tell the two apart.
struct Fix16PointPOD
{
    Fix16PointPOD() = default;

    Fix16PointPOD(Fix16& a1, Fix16& a2)
    {
        x = a1;
        y = a2;
    }

    void SetXY(const Fix16& a2, const Fix16& a3)
    {
        this->x = a2;
        this->y = a3;
    }

    void SetFromPolar(const Fix16& radius, const Ang16& angle)
    {
        x = Ang16::Sine(angle) * radius;
        y = Ang16::Cosine(angle) * radius;
    }

    // Matching impl at RotateVelocity_562C20
    void RotateByAngle(const Ang16& angle)
    {
        Fix16 sin = Ang16::Sine(angle);
        Fix16 cos = Ang16::Cosine(angle);

        Fix16 x_old = x;

        x = (x * cos) + (y * sin);
        y = ((-x_old) * sin) + (y * cos);
    }

    Fix16PointPOD& operator+=(Fix16Point& other);

    // Same layout: a POD passed where a Fix16_Point is expected (a free inline site, like the other way)
    operator Fix16Point&()
    {
        return *reinterpret_cast<Fix16Point*>(this);
    }

    Fix16 x;
    Fix16 y;
};

class Fix16Point
{
  public:
    void SetXY(const Fix16& a2, const Fix16& a3)
    {
        this->x = a2;
        this->y = a3;
    }

    bool IsNull() const
    {
        return x == kFixZero && y == kFixZero;
    }

    bool HasZeroComponent() const
    {
        return x == kFixZero || y == kFixZero;
    }

    void ApplyDeadZone()
    {
        Fix16 total = (Fix16::Abs(x) + Fix16::Abs(y));
        if (total < kFP16One256th_6FE07C)
        {
            x = kFixZero;
            y = kFixZero;
        }
    }

    // But also 0x40ACD0 non inlined in 10.5
    Ang16 Atan2() const
    {
        return Fix16::Atan2Fixed(y, x);
    }

    Fix16 GetLengthOutOfLine();

    // 9.6f 0x41E260; the out-of-line copy is GetLength_453590. The nested if/else (front-end size 162, the
    // else-if chain is 160) gives the original's cut-offs in Crane_15C::ComputeHookPolar_47F6C0 with
    // SquareRoot at 48; matched functions allow 159..162.
    Fix16 GetLength() const
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

    void SetFromPolar(const Fix16& radius, const Ang16& angle)
    {
        x = Ang16::Sine(angle) * radius;
        y = Ang16::Cosine(angle) * radius;
    }

    // Matching impl at RotateVelocity_562C20
    void RotateByAngle(const Ang16& angle)
    {
        Fix16 sin = Ang16::Sine(angle);
        Fix16 cos = Ang16::Cosine(angle);

        Fix16 x_old = x;

        x = (x * cos) + (y * sin);
        y = ((-x_old) * sin) + (y * cos);
    }

    void FromPolar(const Fix16& radius, const Ang16& angle)
    {

        x = radius * Ang16::Sine(angle);
        y = radius * Ang16::Cosine(angle);
    }

    Fix16Point& operator+=(const Fix16Point& other)
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    Fix16Point& operator-=(Fix16Point& other)
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    // Operator* for Fix16 ?
    Fix16Point& MultiplyByFix16(const Fix16& factor)
    {
        x *= factor;
        y *= factor;
        return *this;
    }

    Fix16Point Multiply(const Fix16& in) const;
    Fix16Point Divide(const Fix16& in);

    // Divide_442CB0 as a nothrow inline (see DivideInl_55F9E0; CarPhysics_B0::HandleObjectCollision_5606C0,
    // Car_BC::ApplyExplosionImpulse_443710)
    Fix16Point DivideByFix16(Fix16& in) throw()
    {
        return Fix16Point(x / in, y / in);
    }

    // Multiply_438FE0 as a nothrow inline (see DivideInl_55F9E0; Car_BC::TryHitchTrailer_442810)
    Fix16Point MultiplyInl(Fix16& in) throw()
    {
        return Fix16Point(x * in, y * in);
    }

    // Out-of-line copies emitted in Weapon_30.cpp (used by sub_5DE910).
    Fix16Point& AddAssign(const Fix16Point& other);
    Fix16Point& DivAssign(const Fix16& v);
    Fix16 MaxAbsOutOfLine();

    // 9.6f 0x48A270 (the inline of MaxAbs_5E4140)
    Fix16 MaxAbs()
    {
        Fix16 ax = Fix16::Abs(x);
        Fix16 ay = Fix16::Abs(y);
        if (ax > ay)
        {
            return ax;
        }
        return ay;
    }

    // 9.6f 0x48A250: both components through Fix16::DivideAssign (10.5 0x539F90)
    void DivideAssign(const Fix16& d)
    {
        x.DivideAssign(d);
        y.DivideAssign(d);
    }

    void Reset()
    {
        x = Fix16(0);
        y = Fix16(0);
    }

    // Both inlined and exists as a function... some strange array init behaviour??
    ~Fix16Point() = default;

    // It needs to be in the header
    // 0x563970

    Fix16Point() = default;

    // No user-defined copy ctor: 9.6f copies points with plain movs (no call even at /Ob0), and the
    // implicit one does not use up VC6's inline budget (SpawnCabAndTrailerHelper_408370 needs that).

    Fix16Point(const Fix16& a1, const Fix16& a2) : x(a1), y(a2)
    {
    }

    void ClampTowardsZero(const Fix16Point& limit)
    {
        if (x >= kFixZero)
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

        if (y >= kFixZero)
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
    Fix16Point operator+(const Fix16Point& in)
    {
        return Fix16Point(x + in.x, y + in.y);
    }

    // 0x40AC80
    Fix16Point operator-(const Fix16Point& rhs) const
    {
        return Fix16Point(x - rhs.x, y - rhs.y);
    }

    // The out-of-line copy of the inline operator-. Called by name where the original keeps an EH state
    // for temporaries around the call (Crane_15C::HookPickupCar_47EF80, ComputeHookPolar_47F6C0); the
    // inline operator- called out of line gets none (CarPhysics_B0::HandleCarCollision_55FF20)
    Fix16Point Sub(const Fix16Point& rhs) const;

    // operator+/operator- with their bodies defined at the end of Car_BC.cpp (HandleCarHitByObject_43F130)
    inline Fix16Point AddLate_40AC50(const Fix16Point& in);
    inline Fix16Point SubLate_40AC80(const Fix16Point& rhs);

    // Out of line operator+ (CarPhysics_B0::SpawnSkidSegment_55D200)
    Fix16Point Add(const Fix16Point& in);

    // operator+ 0x40AC50 as a nothrow inline that VC6 still calls out of line: no EH state for the
    // temporaries alive across the call (Car_BC::TryHitchTrailer_442810)
    Fix16Point AddInl(const Fix16Point& in) throw()
    {
        return Fix16Point(x + in.x, y + in.y);
    }

    // Out of line unary minus (Object_2C::ResolveCollisionWithPed_5229B0)
    Fix16Point Negate() const;

    Fix16Point operator*(const Fix16& in)
    {
        return Fix16Point(x * in, y * in);
    }

    Fix16Point operator-() const
    {
        return Fix16Point(-x, -y);
    }

    Fix16Point MultBy(const s32& factor);

    // 10.0 0x442CB0
    Fix16Point operator/(const Fix16& in) const;

    Fix16Point NormalizeSafe();

    Ang16 Atan2OutOfLine();

    Fix16Point Rotate90CCW();

    void Clear()
    {
        x = 0;
        y = 0;
    }

    Fix16Point operator/(const s32& a3);

    // operator/ 0x55F9E0 as a nothrow inline that VC6 still calls out of line: the original sets no EH
    // state for the temporaries that live across this call (CarPhysics_B0::HandleCarCollision_55FF20,
    // HandleObjectCollision_5606C0). throw() on the real operator/ changes 6 matched functions.
    Fix16Point DivideByInt(const s32& a3) throw()
    {
        return Fix16Point(x / a3, y / a3);
    }

    // Same layout: a point sliced into a Fix16_Point_POD (one free inline site, see sub_5DF270)
    operator Fix16PointPOD&()
    {
        return *reinterpret_cast<Fix16PointPOD*>(this);
    }

    Fix16 x;
    Fix16 y;
};

inline Fix16PointPOD& Fix16PointPOD::operator+=(Fix16Point& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

struct Fix16Vec
{
    Fix16 x, y, z;
};
