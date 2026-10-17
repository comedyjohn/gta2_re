#pragma once

#include "Function.hpp"
#include "fix16.hpp"

EXTERN_GLOBAL(class Ang16, kAng180_669156);

class Ang16
{
  public:
    //   Ang16  and  Ang16
    // 9.6f 0x40E670/0x41E0F0/0x40F4C0/0x40E690: setcc into a 32-bit register, so these return int, not bool
    s32 operator>(const Ang16& other) const
    {
        return rValue > other.rValue;
    }

    s32 operator<(const Ang16& other) const
    {
        return rValue < other.rValue;
    }

    s32 operator>=(const Ang16& other) const
    {
        return rValue >= other.rValue;
    }

    s32 operator<=(const Ang16& other) const
    {
        return rValue <= other.rValue;
    }

    s32 operator!=(const Ang16& other)
    {
        return rValue != other.rValue;
    }

    Ang16 operator-=(const Ang16& other)
    {
        rValue -= other.rValue;
        Normalize();
        return *this;
    }

    Ang16 operator+=(const Ang16& other)
    {
        rValue += other.rValue;
        Normalize();
        return *this;
    }

    // TODO: this shouldn't exist, try to match Player::DoPedControlInputs_566C80 without it
    /*
    Ang16 operator=(const Ang16& other)
    {
        rValue = other.rValue;
        return *this;
    }
    */

    s32 operator==(const Ang16& other)
    {
        return rValue == other.rValue;
    }

    Ang16 operator*(const Ang16& other)
    {
        return Ang16((Fix16(rValue) * Fix16(other.rValue)).ToInt());
    }

    Ang16 operator+(const Ang16& rhs)
    {
        s16 value = rValue + rhs.rValue;
        return Ang16(value, 0);
    }

    Ang16 operator-(const Ang16& other)
    {
        s16 value = rValue - other.rValue;
        return Ang16(value, 0);
    }

    // 9.6f 0x49E240, defined in CarPhysics_B0.cpp
    inline wchar_t* ThetaText_49E240();

    Ang16 operator-()
    {
        return Ang16(-rValue, 0);
    }

    //   Ang16  and  const int

    bool operator>(const s32 other) const
    {
        return rValue > other;
    }

    bool operator<(const s32 other) const
    {
        return rValue < other;
    }

    bool operator>=(const s32 other) const
    {
        return rValue >= other;
    }

    bool operator<=(const s32 other) const
    {
        return rValue <= other;
    }

    Ang16 operator-=(const s32 other)
    {
        rValue -= other;
        return *this;
    }

    Ang16 operator+=(const s32 other)
    {
        rValue += other;
        return *this;
    }

    Ang16 operator<<(const s32 other)
    {
        rValue = rValue << other;
        return *this;
    }

    Ang16 operator>>(const s32 other)
    {
        rValue = rValue >> other;
        return *this;
    }

    Ang16 operator*(const s32 other)
    {
        rValue *= other;
        return *this;
    }

    Ang16 operator=(const s32 other)
    {
        rValue = other;
        return *this;
    }

    Ang16 operator-(const s32 other)
    {
        rValue -= other;
        return *this;
    }

    s32 ToFloat()
    {
        return rValue << 14;
    }

    s32 FromUnsignedToFloat()
    {
        return static_cast<u16>(rValue) << 14;
    }

    void ConvertAndMultiply(Ang16* a1, Ang16* a2)
    {
        Fix16 fix_1;
        fix_1.mValue = a2->FromUnsignedToFloat();
        Fix16 fix_2;
        fix_2.mValue = a1->ToFloat();
        Fix16 fix_3;
        fix_3.mValue = fix_2.MultiplyInt64(fix_1);

        rValue = fix_3.ToInt();
    }

    void NormalizeOutOfLine();

    Ang16& Normalized()
    {
        NormalizeOutOfLine();
        return *this;
    }

    void Normalize()
    {
        for (; rValue < 0; rValue += 1440)
        {
            ;
        }
        for (; rValue >= 1440; rValue -= 1440)
        {
            ;
        }
    }

    static bool IsAngleAhead(Ang16* pFrom, Ang16* pTo);

    void SnapToAng4();
    Ang16* AssignNormalized(const s16& value, s32 a3);
    Ang16 SubtractNormalized(const Ang16& toSub);
    static Ang16 Fix16ToAng16OutOfLine(Fix16& value);

    // 10.5 0x409300 (AssignNormalized_409300) is the out-of-line copy of the normalizing constructor
    // Ang16(const s16&, s32) below (9.6f 0x401C60). VC6 calls it instead of inlining in functions that
    // ran out of inline expansions. C++ can't declare the inline and the out-of-line copy of one
    // constructor, so the out-of-line one is reached through these overloads.
    Ang16(Ang16& input, s32 a3)
    {
        AssignNormalized(input.rValue, a3);
    }

    Ang16(const s16* value, s32 a3)
    {
        AssignNormalized(*value, a3);
    }

    // The value is stored as a dword, as 9.6f's 0x40F540 passes it to 0x401C60; the copy reads its low word
    Ang16(const s32* value, s32 a3)
    {
        AssignNormalized(*reinterpret_cast<const s16*>(value), a3);
    }

    // The out-of-line copy of Ang16(Fix16& value, u8) (9.6f 0x401C40). A real constructor, not a
    // method: when it is called out of line its 0 argument is pushed before the Fix16 argument is
    // evaluated (Particle_8::EmitImpactParticles_53FE40). Also used by miss2_0x11C::SCRCMD_CRANE_5041C0.
    Ang16(Fix16* a2, s32 a3);

    static Fix16 Cosine(const Ang16& angle)
    {
        return gCos_table_669260[angle.rValue];
    }

    static Fix16 Sine(const Ang16& angle)
    {
        return gSin_table_667A80[angle.rValue];
    }

    // 9.6f 0x41FC20: the table value is the left operand (this) of the multiply
    static void PolarToCartesian(const Ang16& angle, const Fix16& radius, Fix16& ret1, Fix16& ret2)
    {
        ret1 = Ang16::Sine(angle) * radius;
        ret2 = Ang16::Cosine(angle) * radius;
    }

    // Non-inlined version of above function
    static void PolarToCartesianOutOfLine(Ang16& angle, const Fix16& radius, Fix16& ret1, Fix16& ret2);

    static s32 GetAngleFace(const Ang16& angle);

    // inlined from 0x40f580 on v9.6f
    // I was tempted to put this on the Fix16 header, but that will create a circular dependency...
    static Fix16 _stdcall Ang16_to_Fix16(const Ang16& v)
    {
        s32 value = v.rValue * 71;
        return Fix16(value, 0);
    }

    // Reverse of above func
    static Ang16 Fix16ToAng16(const Fix16& a2)
    {
        return Ang16(a2.GetRaw() / 71, 0);
    }

    bool IsAxisAligned() const
    {
        return (rValue == 0 || rValue == 360 || rValue == 720 || rValue == 1080) ? true : false;
    }

    bool JIsAxisAligned()
    {
        return IsAxisAligned();
    }

    Ang16(const s16& value, s32 not_used) : rValue(value)
    {
        this->Normalize();
    }

    Ang16() : rValue(0)
    {
    }

    Ang16(s16 value, u8 null2) : rValue(value)
    {
        Normalize();
    }

    Ang16(s32 value) : rValue(value)
    {
    }

    Ang16(Fix16 value) : rValue(value.mValue >> 14) // value.ToInt() breaks Map_0x370::SpawnMapObjects_4DFB90
    {
    }

    // 9.6f func: 0x401C40
    Ang16(const Fix16& value, u8 zero)
    {
        rValue = value.ToInt();
        Normalize();
    }

    Ang16 MultiplyByFix16(const Fix16& a2)
    {
        return Ang16(Fix16(rValue) * a2, 0);
    }

    Fix16 DivideBy(const Ang16& scale) const
    {
        return Fix16(rValue) / scale.rValue;
    }

    static Fix16 NormalizeAngleDeltaScaled(Ang16& from, Ang16& to, Ang16& divisor);
    u8 GetOctant();
    u8 ToAng4();

    // TODO: move with CarDoorAlignmentSolver_545AF0
    // https://decomp.me/scratch/6VBR0  (matched)
    static void RotateVector(Fix16& xpos, Fix16& ypos, Ang16& rotation)
    {
        Fix16 old_xpos = xpos;
        xpos = xpos * Ang16::Cosine(rotation) + ypos * Ang16::Sine(rotation);
        ypos = (-old_xpos) * Ang16::Sine(rotation) + ypos * Ang16::Cosine(rotation);
    }

    s16 rValue;
};

inline Ang16 operator-(const Ang16& a, const Ang16& b)
{
    return a.rValue - b.rValue;
}

class Ang8
{
  public:
    //  maybe an Ang8 inline at 0x462EF0 in 9.6f
    Ang16 convert_to_ang16()
    {
        return Ang16((Fix16(rValue) / 256) * 1440);
    }

    u8 rValue;
};

inline void RotateAndTranslatePoint(const Fix16& pInX,
                                    const Fix16& pInY,
                                    const Ang16& pRotAng,
                                    Fix16& pTransX,
                                    Fix16& pTransY,
                                    Fix16& pRotTransX,
                                    Fix16& pRotTransY)
{
    pRotTransX = (((pInX - pTransX) * Ang16::Cosine(pRotAng)) + ((pInY - pTransY) * Ang16::Sine(pRotAng)));
    pRotTransY = ((-(pInX - pTransX) * Ang16::Sine(pRotAng)) + ((pInY - pTransY) * Ang16::Cosine(pRotAng)));
}
