#pragma once

#include "core/function.hpp"
#include <math.h>
#include <windows.h>

class Fix16;

EXTERN_GLOBAL(Fix16, kFixZero);
EXTERN_GLOBAL(Fix16, kFP16One256th_6FE07C);
EXTERN_GLOBAL(Fix16, kNegligibleValue);

class Fix16
{
  public:
    // 9.6f func
    static s32 RoundToInt(const Fix16& a1)
    {
        s32 v = a1.value_;
        return (v + 0x2000) >> 14;
    }

    s32 operator==(const Fix16& value) const
    {
        return value_ == value.value_;
    }

    s32 operator<=(const Fix16& other) const
    {
        return value_ <= other.value_;
    }

    Fix16& operator=(s32 value)
    {
        value_ = value;
        return *this;
    }

    Fix16 operator-(const Fix16& in) const
    {
        s32 value = value_ - in.value_;
        return Fix16(value, 0);
    }

    Fix16& operator-=(const Fix16& other)
    {
        value_ -= other.value_;
        return *this;
    }

    Fix16 operator+(const Fix16& in)
    {
        s32 value = value_ + in.value_;
        return Fix16(value, 0);
    }

    Fix16& operator+=(const Fix16& other)
    {
        value_ += other.value_;
        return *this;
    }

    Fix16 operator*(const Fix16& in) const
    {
        s32 value = static_cast<s32>((value_ * static_cast<__int64>(in.value_)) >> 14);
        return Fix16(value, 0);
    }

    // 10.5 non inline addr is 0x539F90
    Fix16& operator/=(const Fix16& rhs)
    {
        value_ = static_cast<s32>((static_cast<__int64>(value_) << 14) / rhs.value_);
        return *this;
    }

    // 10.5 non inline addr is 0x562430
    Fix16& operator*=(const Fix16& rhs)
    {
        value_ = static_cast<s32>((value_ * static_cast<__int64>(rhs.value_)) >> 14);
        return *this;
    }

    // 10.5 is 0x561DB0
    // Inlined from 9.6f from 0x401bd0
    Fix16 operator*(const s32& in) const
    {
        s32 value = value_ * in;
        return Fix16(value, 0);
    }

    // 0x4086A0
    Fix16 operator-() const
    {
        return Fix16(-value_, 0);
    }

    s32 operator>(const Fix16& other) const
    {
        return value_ > other.value_;
    }

    s32 operator<(const Fix16& other) const
    {
        return value_ < other.value_;
    }

    s32 operator!=(const Fix16& other) const
    {
        return value_ != other.value_;
    }

    s32 operator>=(const Fix16& other) const
    {
        return value_ >= other.value_;
    }

    // 0x509990
    bool operator>=(const s32 value) const
    {
        return value_ >= value << 14;
    }

    // 0x509990
    bool operator<(const s32 value) const
    {
        return value_ < value << 14;
    }

    f32 AsFloat() const
    {
        return value_ / 16384.0F;
    }

    f64 AsDouble() const
    {
        return value_ / 16384.0;
    }

    s32 ToInt() const
    {
        return value_ >> 14;
    }

    u8 ToUInt8() const
    {
        return value_ >> 14;
    }

    void FromInt(s32 a1)
    {
        value_ = a1 << 14;
    }

    void FromShort(s16 a1)
    {
        value_ = a1 << 14;
    }

    void FromUnsignedShort(u16 a1)
    {
        value_ = a1 << 14;
    }

    float ToFloat() const
    {
        return (value_ / 16384.0F);
    }

    // Zero rather than unset: the original reads some Fix16 locals before setting them, and every
    // machine in a network game has to read the same value
    constexpr Fix16() : value_(0)
    {
    }

    Fix16(s32 value, u8) : value_(value)
    {
    }

    Fix16(u8 value)
    {
        value_ = value << 14;
    }

    Fix16(u16 value)
    {
        value_ = value << 14;
    }

    Fix16(s16 value)
    {
        value_ = value << 14;
    }

    Fix16(u32 value)
    {
        value_ = value << 14;
    }

    Fix16(s32 value)
    {
        value_ = value << 14;
    }

    explicit Fix16(f32 v) : value_(static_cast<s32>(v * 16384.0F))
    {
    }

    explicit Fix16(f64 v) : value_(static_cast<s32>(v * 16384.0))
    {
    }

    void FromU8(u8 v)
    {
        value_ = v << 14;
    }

    Fix16& Negate()
    {
        value_ = -value_;
        return *this;
    }

    // 9.6f 0x403840, out-of-line copy Abs_436A50. No braces and a const& parameter on purpose: VC6's
    // inline budget sees the front-end size, 57 here (each brace pair adds 2, the else 2 more, a non-const
    // parameter 1 less). 57 is the only size that gives the original's cut-offs both in MaxAbsDistance_42A6B0
    // (Ped::NotifyWeaponHit_46FF00 needs <= 57) and in GetLength_41E260 (Car_BC::ManageDrowning_43E560 >= 57).
    static Fix16 Abs(const Fix16& input)
    {
        if (input.value_ > 0)
        {
            return input;
        }
        return -input;
    }

    Fix16 ZeroIfNegligible()
    {
        if (Fix16::Abs(*this) < kNegligibleValue)
        {
            return kFixZero;
        }
        return *this;
    }

    Fix16 GetRoundValue() const
    {
        // get the "integer part" of Fix16, since everything less than 0x3FFF is decimal in float
        return Fix16(value_ & 0xFFFFC000, 0); // 0xFFFFC000 = 0xFFFFFFFF - Fix16(1)
    }

    // 9.6f 0x42A630 as a static taking the value by reference; defined in CarPhysics_B0.cpp (UpdateZPhysics_55AD90)
    // 9.6f func: 0x42A630
    Fix16 GetFracValue() const
    {
        // get the "fractional part" of Fix16
        return Fix16(value_ & 0x3FFF, 0); // 0x4000 = Fix16(1)
    }

    s32 MultiplyInt64(Fix16 a2)
    {
        __int64 t = (value_ * static_cast<__int64>(a2.value_)) >> 14;
        return static_cast<s32>(t);
    }

    //  inline sub_4B9E10 in 9.6f
    static bool IsBetween(Fix16& min, Fix16& max, Fix16& input)
    {
        return input >= min && input <= max;
    }

    //  inline sub_462ED0 in 9.6f
    static Fix16 Ctor(s16 a1)
    {
        return Fix16(a1 << 7, 0);
    }

    // Non-member style add of two references. Unlike the member operator+ it loads the left
    // operand into the result register first (CarPhysics_B0::ComputeEngineTorque_561970's length).
    static Fix16 AddRef(const Fix16& lhs, const Fix16& rhs)
    {
        return Fix16(lhs.value_ + rhs.value_, 0);
    }

    static Fix16 Max(const Fix16& diff_x, const Fix16& diff_y)
    {
        return (diff_x > diff_y) ? diff_x : diff_y;
    }

    // 9.6f 0x410C10, out-of-line copy SquareRoot_436A70. A plain inline: big functions call 0x436A70 once
    // they run out of inline budget (Car_BC::ManageDrowning_43E560). The const local is on purpose: VC6's
    // inline budget charges the front-end size, 48 here (41 without the local, 46 with a non-const one).
    // Car_BC::ApplyExplosionImpulse_443710 needs >= 48 and CarPhysics_B0::EnforceGearSensitiveMaxSpeed_562D00
    // <= 49; with GetLength_41E260 at 162, Crane_15C::ComputeHookPolar_47F6C0 needs >= 48 too.
    static Fix16 SquareRoot(const Fix16& input)
    {
        const f64 value = input.AsDouble();
        return Fix16(sqrt(value));
    }

    // throw(): the original calls these out-of-line copies without an EH frame (their inline
    // bodies were visible there), see CarPhysics_B0::UpdateReferencePoint_563460
    // Out-of-line copy of operator+ (0x408660)
    Fix16 Add(const Fix16& rhs) const throw();
    Fix16 Multiply(const Fix16& in) const throw();
    // Out-of-line copies of operators, which big functions call once they run out of inline
    // expansions (SpriteShape_4C::DrawCollisionBox_5A4DA0)
    Fix16 Subtract(const Fix16& in) const;
    Fix16 Divide(const Fix16& in) const;
    s32 IsLess(const Fix16& other) const;
    s32 IsGreater(const Fix16& other) const;
    Fix16& DivideAssign(const Fix16& rhs);
    Fix16 MultiplyInt(const s32& in) const;
    Fix16 DivideInt(const s32& in) const;
    Fix16& MultiplyAssign(const Fix16& rhs);
    Fix16 Negated() const throw();

    // 0x436A20
    Fix16 operator/(const Fix16& in) const
    {
        s32 value = static_cast<s32>((static_cast<__int64>(value_) << 14) / in.value_);
        return Fix16(value, 0);
    }

    // Inlined from 9.6f at 0x401bf0
    // I am not fully sure if this is right, i.e. the s32 parameter, instead of Fix16.
    // But I couldn't match ObjectDefinition_74::SetDimensionsFromSprite_533090 without this overload.
    Fix16 operator/(const s32& in)
    {
        s32 value = value_ / in;
        return Fix16(value, 0);
    }

    s32 GetValue() const
    {
        return value_;
    }

    s32 GetRaw() const
    {
        return value_;
    }

    Fix16 ApplyDeadZone(Fix16 to_abs)
    {
        if (!(Fix16::Abs(to_abs) < kFP16One256th_6FE07C))
        {
            return to_abs;
        }
        else
        {
            return kFixZero;
        }
    }

    static Fix16 MaxAbsDistance(const Fix16& x1, const Fix16& y1, const Fix16& x2, const Fix16& y2)
    {
        Fix16 diff_x;
        diff_x = x2 - x1;
        Fix16 diff_y;
        diff_y = y2 - y1;
        Fix16 result;
        result = Fix16::Max(Fix16::Abs(diff_x), Fix16::Abs(diff_y));
        return result;
    }

    // NOTE: 10.5 function - matched but inlined
    static Fix16 ClampToRangeFlexible(const Fix16& a2, const Fix16& a3, const Fix16& a4)
    {
        if (a2 > a3)
        {
            if ((a2 <= a4))
            {
                return a4;
            }
            else
            {
                return a2;
            }
        }
        else
        {
            if (a3 > a4)
            {
                return a3;
            }
            else
            {
                return a4;
            }
        }
    }

    Fix16* SubtractOne()
    {
        value_ -= 0x4000;
        return this;
    }

    Fix16* AddOne()
    {
        value_ += 0x4000;
        return this;
    }

    Fix16 operator++(int)
    {
        value_ += 0x4000;
        return Fix16(value_ - 0x4000, 0);
    }

    Fix16 operator--(int)
    {
        value_ -= 0x4000;
        return Fix16(value_ + 0x4000, 0);
    }

    static class Ang16 Atan2Fixed(const Fix16& y, const Fix16& x);

    s32 value_;
};

bool IntervalIntersectsRange(const Fix16& intervalStart, const Fix16& intervalEnd, const Fix16& rangeMin, const Fix16& rangeMax);

class Ang16;
Ang16 ArcTanLookup(const Fix16& targetTan);

void FindMinMax(Fix16& minOut, Fix16& maxOut, const Fix16& a, const Fix16& b, const Fix16& c, const Fix16& d);

Fix16 TurnAngleToward(Fix16 cur, Fix16* pTarget, Fix16* pSpeed);
s32 ClampAngleToWindow(Fix16* pTarget, Fix16* pCur);
EXTERN_GLOBAL_ARRAY(Fix16, gSinTable, 1440);
EXTERN_GLOBAL_ARRAY(Fix16, gCosTable, 1440);

void InitTrigonometryTables();
